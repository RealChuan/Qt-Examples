#pragma once

#include <LifecycleCallback/lifecyclecallback.hpp>

#include <QByteArrayView>
#include <QDateTime>
#include <QHttpHeaders>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslCertificate>
#include <QSslError>
#include <QSslKey>

#include <chrono>
#include <memory>
#include <optional>

class HttpClient;
class RequestBuilder;

/**
 * @brief 错误码
 *
 * 值域：0-999 复用 QNetworkReply::NetworkError（含服务器错误状态的映射码，
 * 如 404 -> ContentNotFoundError），1000 起为本地扩展码。
 */
enum class HttpErrorCode : int {
    NoError = 0,
    Timeout = 1000,   // 请求总时长超时
    FileError = 1001, // 上传源文件或下载目标文件打开/写入/保存失败
    InvalidUse = 1002, // API 误用：重复终止、移动后的构建器、upload + body()/json() 冲突、upload 未配置载荷
    Canceled = 1003, // QNAM 内部取消（HTTP/2 RST_STREAM 等）；用户 cancel() 不产生结果
};

/**
 * @brief 请求终止结果
 *
 * 所有终止路径（成功、服务器错误状态、网络失败、超时）统一产出本结构：
 *  - success()                    -> 无协议错误：2xx（json 可直接使用）、条件请求命中的
 *                                   304（body 为空，下载时既有文件不受影响）、未跟随的 3xx
 *  - !success() && status >= 400  -> 服务器错误状态（4xx/5xx），body/json 是 API 错误详情
 *  - !success() && status < 400   -> 传输失败（status 为已收到的状态码，头未到达则为 0）、
 *                                   超时/文件错误/误用/QNAM 内部取消
 */
struct HttpResult
{
    HttpErrorCode code = HttpErrorCode::NoError;
    QString message;    // 失败原因：QNAM errorString() 或本地映射消息
    QUrl url;           // 请求 URL（重定向跟随后的最终地址）
    int status = 0;     // HTTP 状态码；0 表示服务器无响应
    QByteArray body;    // 原始响应体；下载成功时为空（数据已写入文件）
    QJsonDocument json; // body 为合法 JSON 时即已解析，否则为 null
    QByteArray
        method; // 请求动词（枚举动词规范大写，字符串动词原样保留）；入口检查未通过（moved-from/二次使用）时为空
    QList<QSslError> sslErrors;
    QHttpHeaders headers; // 全部响应头（多值头如 Set-Cookie 保留多条）

    Q_REQUIRED_RESULT bool success() const { return code == HttpErrorCode::NoError; }

    /**
     * @brief 大小写不敏感地取该响应头的全部值（多值头如 Set-Cookie 必需）；
     *        未命中返回空列表。单值头返回单元素列表。
     */
    Q_REQUIRED_RESULT QList<QByteArray> header(QByteArrayView name) const;
};

using HttpFinishedCallback = LifecycleCallback<const HttpResult &>;
using HttpProgressCallback = LifecycleCallback<qint64, qint64>;

/**
 * @brief 异步请求句柄，仅用于取消
 *
 * 可拷贝（拷贝共享同一任务，存入成员/容器后多处触发取消）；
 * 丢弃句柄不影响请求执行（fire-and-forget）。
 * cancel() 后 onFinished 不再派发：调用方主动终止请求，无需收尾通知。
 * 下载场景保留 .temp 文件供下次断点续传。
 */
class HttpTask
{
public:
    HttpTask() = default;
    HttpTask(const HttpTask &) = default;
    HttpTask &operator=(const HttpTask &) = default;
    ~HttpTask() = default;

    /**
     * @brief 取消请求。幂等；对已终止或已完成的请求为安全空操作。
     */
    void cancel() const;

private:
    friend class HttpClient;

    HttpTask(HttpClient *client, QNetworkReply *reply);

    class HttpTaskPrivate;
    std::shared_ptr<HttpTaskPrivate> d_ptr;
};

/**
 * @brief 通用 HTTP 客户端
 *
 * 分层设计：
 *  - 通用层：RequestBuilder::body(QByteArray, contentType) + method(QByteArrayView)，
 *    覆盖任意动词与任意载荷（XML/protobuf/二进制/空体/扩展动词等）
 *  - 便利层：json()/post(url, obj)/put(url, obj) 等以 JSON 语法糖转调通用层
 *  - 响应侧对应便利：HttpResult::json
 *
 * 继承 QNetworkAccessManager：连接池/代理/cookie 等原生能力可用，
 * 子类可 override createRequest() 注入全局策略（统一 header、
 * QNetworkRequest::Attribute、按 URL 分流的 SSL/代理等）；所有请求路径均经过此处。
 * 响应侧对称扩展点为 resultReady()：结果装配完成、用户回调派发前调用，
 * 子类可观察/加工所有经 dispatch 的结果。
 *
 * 基类原始调用（如 get(QNetworkRequest)）返回裸 QNetworkReply*，
 * 不参与本类的回调/超时/取消管理（Basic 凭证的挑战-响应回填除外，
 * 见 RequestBuilder::basicAuth）。
 */
class HttpClient : public QNetworkAccessManager
{
    Q_OBJECT

public:
    enum class Method : int { Get, Post, Put, Delete, Patch };
    Q_ENUM(Method)

    explicit HttpClient(QObject *parent = nullptr);
    ~HttpClient() override;

    using QNetworkAccessManager::get;
    using QNetworkAccessManager::post;
    using QNetworkAccessManager::put;

    // ── 请求工厂（无 body 版）──
    Q_REQUIRED_RESULT RequestBuilder get(QUrl url);
    Q_REQUIRED_RESULT RequestBuilder post(QUrl url);                   // POST 无 body
    Q_REQUIRED_RESULT RequestBuilder put(QUrl url);                    // PUT 无 body
    Q_REQUIRED_RESULT RequestBuilder del(QUrl url);
    Q_REQUIRED_RESULT RequestBuilder request(Method method, QUrl url); // 无 body

    // ── 请求工厂（JSON body 便利版：空对象序列化为 "{}"）──
    Q_REQUIRED_RESULT RequestBuilder post(QUrl url, const QJsonObject &json);
    Q_REQUIRED_RESULT RequestBuilder put(QUrl url, const QJsonObject &json);
    Q_REQUIRED_RESULT RequestBuilder request(Method method, QUrl url, const QJsonObject &json);

    /**
     * @brief 下载到文件。写入 <filePath>.temp，成功后改名；
     *        已有 .temp 时自动带 Range 头断点续传。
     *        默认 GET；经链式 method()/body()/json() 可改写动词与请求体：
     *          - .method(Post)                -> POST 无 body 下载
     *          - .method(Post).json(obj)      -> POST + JSON 动作体下载
     *          - .method("REPORT").json(obj) -> 扩展动词 + JSON body
     *
     *        Range 头由 download() 独占管理：调用方经 header() 设置的 Range
     *        一律被忽略（含无 .temp 的首次下载）。
     */
    Q_REQUIRED_RESULT RequestBuilder download(QUrl url, QString filePath);

    /**
     * @brief 上传载荷。默认 PUT，经 method() 可改写动词：
     *          - putFile / putData     原始体，默认 PUT，可改写为 POST 等
     *          - multipartFile / Data  POST multipart/form-data（恒为 POST：
     *                                  QNAM 无自定义动词的 multipart 重载，
     *                                  改写动词会得到告警并被忽略）
     *
     *        载荷由 putFile/putData/multipart* 独占：再调 body()/json() 属
     *        API 误用，prepare 时报 InvalidUse（而非静默忽略）。
     */
    Q_REQUIRED_RESULT RequestBuilder upload(QUrl url);

protected:
    /**
     * @brief 响应侧扩展点：与 createRequest()（请求侧）对称。
     *
     * 结果装配完成（含 body 解析/下载落盘）、用户 onFinished 派发之前调用，
     * 故 override 内发起的后续请求已可见本结果的全部效应。
     * 覆盖 send()/sync() 发起请求的所有终止路径：成功/服务器错误/网络失败/
     * 超时/本地失败（文件打开失败等）。send()/sync() 入口的参数误用
     * （moved-from、二次使用）与基类原始调用不经过此处。
     * 实现须快速返回，勿在内部发起同步请求（回调栈内嵌套事件循环）。
     */
    virtual void resultReady(const HttpResult &result);

private:
    friend class RequestBuilder;
    friend class HttpTask;

    HttpTask execute(RequestBuilder &builder);
    HttpResult executeSync(RequestBuilder &builder);
    void cancelInternal(QNetworkReply *reply);

private slots:
    void onReplyFinished();
    void onSslErrors(const QList<QSslError> &errors);
    void onTimeout();
    void onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *auth);
    void onUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void onDownloadReadyRead();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

private:
    class HttpClientPrivate;
    std::unique_ptr<HttpClientPrivate> d_ptr;
};

/**
 * @brief 链式请求构建器
 *
 * 由 HttpClient 的入口函数创建：client.get(url).timeout(30s).send()
 *
 * 请求体为三态模型：
 *  - body(QByteArray, contentType) 通用层：任意字节（可为空字节 = 空体）
 *  - json(QJsonObject)             便利层：序列化后转调 body()
 *  - 两者皆未调用                  = 无 body（get()/del()/post(url) 等入口）
 *
 * Content-Type 是普通请求头：body()/json() 以非空 contentType 写入，
 * header() 直接写入，共用同一覆盖语义（同名大小写不敏感替换），
 * 链上最后一次调用生效。
 *
 * send()/sync() 为终止操作，仅可调用一次（重复调用：send() 返回空句柄并告警，
 * sync() 返回 InvalidUse 错误）。构建器仅可移动；移动后的原对象上一切配置
 * 方法为安全空操作，send()/sync() 同样告警/报错而不崩溃。
 */
class RequestBuilder
{
public:
    RequestBuilder(RequestBuilder &&) noexcept;
    RequestBuilder &operator=(RequestBuilder &&) noexcept;
    ~RequestBuilder();

    RequestBuilder(const RequestBuilder &) = delete;
    RequestBuilder &operator=(const RequestBuilder &) = delete;

    // ── 通用配置 ──
    /**
     * @brief 设置请求头（覆盖语义：同名大小写不敏感替换，非追加）。
     *        多值头请用逗号分隔的单个值表达。
     */
    RequestBuilder &header(QByteArray name, QByteArray value);
    /**
     * @brief Bearer 认证：设置 Authorization: Bearer <token>。
     */
    RequestBuilder &bearer(QByteArrayView token);
    /**
     * @brief Basic 认证：设置 Authorization: Basic base64(user:password)，
     *        并将凭证注册为该 host 的 manager 级状态（同 host 覆盖、不可清除）：
     *        同 host 后续请求遇 401 挑战时自动回填，跨 host 重定向不泄漏；
     *        外部 authenticationRequired 槽在内部槽之后执行，可覆盖回填值；
     *        须在 client 所属线程调用。
     *
     * 挑战-响应路径对 Digest 同样生效：QAuthenticator 依据服务器挑战方式
     * 自动选择应答 scheme，凭证表两种通用。
     */
    RequestBuilder &basicAuth(const QString &user, const QString &password);
    RequestBuilder &query(const QString &key, const QString &value);

    // ── 条件请求（If-* 头便利封装）──
    /**
     * @brief If-None-Match：ETag 与服务器当前值匹配时回 304（缓存校验）。
     *        etag 原样传输：传服务器返回的完整值（含引号 / W 前缀，"*" 表示任意）。
     */
    RequestBuilder &ifNoneMatch(QByteArray etag);
    /**
     * @brief If-Match：ETag 不匹配（含资源已不存在）时服务器回 412，
     *        用于 PUT 覆盖前的丢失更新防护。etag 语义同 ifNoneMatch()。
     */
    RequestBuilder &ifMatch(QByteArray etag);
    /**
     * @brief If-Modified-Since：HTTP 日期（IMF-fixdate，UTC）表达的时间条件，
     *        与 ifNoneMatch() 组合时 ETag 优先。
     */
    RequestBuilder &ifModifiedSince(const QDateTime &lastModified);
    /**
     * @brief If-Unmodified-Since：资源在给定时间后被修改则回 412。
     */
    RequestBuilder &ifUnmodifiedSince(const QDateTime &lastModified);

    /**
     * @brief 改写请求动词（枚举版本）。清除此前设置的字符串动词覆盖。
     */
    RequestBuilder &method(HttpClient::Method method);
    /**
     * @brief 改写请求动词为任意字符串（如 "PROPFIND"、"REPORT"），
     *        扩展协议动词由此表达。与枚举版本互斥，最后一次调用生效；
     *        动词原样使用，不做大小写改写（RFC 9110 §9.1：method 大小写敏感）；
     *        非 RFC 9110 token（含空格/空串等）的输入告警并忽略。
     */
    RequestBuilder &method(QByteArrayView verb);
    /**
     * @brief 通用请求体：任意字节 + Content-Type。
     *
     * data 允许为空（空体是合法 body，如 body("", "text/plain")）；
     * contentType 非空时以覆盖语义设置 Content-Type（链上最后一次写入生效，
     * 其后的 header() 仍可覆盖）；为空时不动 Content-Type，
     * 交由 header() 自行设置。全链未设置任何 CT 时，带体请求的 CT 由
     * QNAM 传输层默认为 application/x-www-form-urlencoded（并告警）。
     */
    RequestBuilder &body(QByteArray data, QByteArray contentType);
    /**
     * @brief JSON body 便利入口：等价于
     *        body(QJsonDocument(obj).toJson(), "application/json; charset=utf-8")。
     *        空对象序列化为 "{}"（有效 JSON body，与"无 body"严格区分）。
     */
    RequestBuilder &json(const QJsonObject &obj);
    RequestBuilder &timeout(std::chrono::milliseconds timeout); // 请求总时长上限，0 为不限
    RequestBuilder &ignoreSslErrors();
    /**
     * @brief mTLS 客户端证书链：链首为叶证书，其后为中级 CA（随 TLS
     *        Certificate 消息一并发送；单证书传 {cert} 即可）。合并进本
     *        请求的 TLS 配置，既有字段（自定义 CA、协议版本等）保留；
     *        链上最后一次调用生效。空链、含空证书或空私钥告警并忽略。
     */
    RequestBuilder &clientCert(const QList<QSslCertificate> &certChain, const QSslKey &key);
    /**
     * @brief 下载专用：覆盖读缓冲在途窗口（背压）。0 或不设置用平台默认
     *        （32 位 4 MiB / 64 位 8 MiB）。并发多取小（内存 ≈ 窗口 × 并发数），
     *        单大文件取大。
     */
    RequestBuilder &readBufferLimit(qint64 limit);

    /**
     * @brief 终止回调。请求结束时触发一次（成功/失败/超时均触发；主动取消不触发）。
     */
    RequestBuilder &onFinished(HttpFinishedCallback callback);

    /**
     * @brief 进度回调，仅上传/下载请求有意义。
     */
    RequestBuilder &onProgress(HttpProgressCallback callback);

    // ── 上传载荷（upload() 后四选一；未配置时结果为 InvalidUse）──
    RequestBuilder &putFile(QString filePath);       // PUT 原始体，文件流式发送不占内存
    RequestBuilder &putData(QByteArray data);        // PUT 原始体
    RequestBuilder &multipartFile(QString filePath); // POST multipart/form-data
    RequestBuilder &multipartData(QString fileName, QByteArray data); // POST multipart

    // ── 终止操作 ──
    /**
     * @brief 异步发起请求，立即返回。回调在 client 所属线程的事件循环派发。
     */
    HttpTask send();

    /**
     * @brief 同步发起请求，阻塞直至结束。结果经返回值交付：忽略 onFinished
     *        （已配置时告警，避免双重交付）；onProgress 正常派发——阻塞期间
     *        事件循环在运行，进度信号照常回调，适用于同步下载大文件的进度上报。
     *
     * 必须在 client 所属线程（通常是主线程）调用。
     * 未设置 timeout 且服务器无响应时会一直阻塞，调用方自行保证可终止。
     */
    Q_REQUIRED_RESULT HttpResult sync();

private:
    friend class HttpClient;
    friend class HttpClient::HttpClientPrivate;

    enum class Kind { Json, Download, Upload };
    enum class PayloadKind { None, PutFile, PutData, MultipartFile, MultipartData };

    RequestBuilder(HttpClient *client,
                   Kind kind,
                   HttpClient::Method method,
                   QUrl url,
                   std::optional<QByteArray> body,
                   QByteArray contentType,
                   QString downloadPath);

    class RequestBuilderPrivate;
    std::unique_ptr<RequestBuilderPrivate> d_ptr;
};
