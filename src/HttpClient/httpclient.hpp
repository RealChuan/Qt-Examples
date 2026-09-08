#pragma once

#include <LifecycleCallback/lifecyclecallback.hpp>

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>

#include <chrono>
#include <memory>

class HttpClient;
class RequestBuilder;

/**
 * @brief 错误码
 *
 * 值域设计：0-999 直接复用 QNetworkReply::NetworkError 的值
 * （网络层失败，以及服务器返回错误状态时 QNAM 的映射码，如 404 -> ContentNotFoundError），
 * 1000 起为本地扩展码，code >= 1000 即本地错误。
 */
enum class HttpErrorCode : int {
    NoError = 0,
    Timeout = 1000,    // 请求总时长超时（QTimer 主动 abort）
    FileError = 1001,  // 上传源文件或下载目标文件打开/写入/保存失败
    InvalidUse = 1002, // API 误用：构建器已 send/sync 后再次终止
};

/**
 * @brief 请求终止结果
 *
 * 所有终止路径（成功、服务器错误状态、网络失败、超时）统一产出本结构：
 *  - success()                 -> 2xx，json 可直接使用
 *  - !success() && status > 0  -> 服务器有响应（4xx/5xx），body/json 是 API 错误详情
 *  - !success() && status == 0 -> 请求未到达服务器（网络失败/超时/文件错误）
 */
struct HttpResult
{
    HttpErrorCode code = HttpErrorCode::NoError;
    QString message;    // 失败原因：QNAM errorString() 或本地映射消息
    QUrl url;           // 请求 URL
    int status = 0;     // HTTP 状态码；0 表示服务器无响应
    QByteArray body;    // 原始响应体；下载成功时为空（数据已写入文件）
    QJsonDocument json; // body 为合法 JSON 时即已解析，否则为 null
    QList<QSslError> sslErrors;
    QList<QNetworkReply::RawHeaderPair> headers;

    Q_REQUIRED_RESULT bool success() const { return code == HttpErrorCode::NoError; }

    /**
     * @brief 大小写不敏感地取首个命中的响应头；未找到返回空。
     */
    Q_REQUIRED_RESULT QByteArray header(QByteArrayView name) const;
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

    class Private;
    std::shared_ptr<Private> d_ptr;
};

/**
 * @brief 以 JSON 为中心的 HTTP 客户端
 *
 * 继承 QNetworkAccessManager：连接池/代理/cookie 等原生能力可用，
 * 子类可 override createRequest() 扩展特殊请求，基类原始重载经 using 保持可见。
 *
 * 基类原始调用（如 get(QNetworkRequest)）返回裸 QNetworkReply*，
 * 不参与本类的回调/超时/取消管理。
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

    Q_REQUIRED_RESULT RequestBuilder get(QUrl url);
    Q_REQUIRED_RESULT RequestBuilder post(QUrl url, const QJsonObject &json = {});
    Q_REQUIRED_RESULT RequestBuilder put(QUrl url, const QJsonObject &json = {});
    Q_REQUIRED_RESULT RequestBuilder del(QUrl url);
    Q_REQUIRED_RESULT RequestBuilder request(Method method, QUrl url, const QJsonObject &json = {});
    /**
     * @brief 下载到文件。写入 <filePath>.temp，成功后改名；
     *        已有 .temp 时自动带 Range 头断点续传。
     */
    Q_REQUIRED_RESULT RequestBuilder download(QUrl url, QString filePath);
    Q_REQUIRED_RESULT RequestBuilder upload(QUrl url);

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
 * 链式配置全部可选；send()/sync() 为终止操作，触发请求，仅可调用一次
 * （重复调用：send() 返回空句柄并告警，sync() 返回 InvalidUse 错误）。
 * 仅可移动；配置状态经 Pimpl 隐藏，不进入公共头文件。
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
    RequestBuilder &header(QByteArray name, QByteArray value);
    /**
     * @brief Bearer 认证：设置 Authorization: Bearer <token>。
     */
    RequestBuilder &bearer(QByteArrayView token);
    /**
     * @brief Basic 认证：设置 Authorization: Basic base64(user:password)。
     */
    RequestBuilder &basicAuth(const QString &user, const QString &password);
    RequestBuilder &query(const QString &key, const QString &value);
    RequestBuilder &timeout(std::chrono::milliseconds timeout); // 请求总时长上限，0 为不限
    RequestBuilder &ignoreSslErrors();

    /**
     * @brief 终止回调。请求结束时触发一次（成功/失败/超时均触发；主动取消不触发）。
     */
    RequestBuilder &onFinished(HttpFinishedCallback callback);

    /**
     * @brief 进度回调，仅上传/下载请求有意义。
     */
    RequestBuilder &onProgress(HttpProgressCallback callback);

    // ── 上传载荷（upload() 后四选一；未配置时结果为 FileError）──
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
     * @brief 同步发起请求，阻塞直至结束。忽略 onFinished/onProgress。
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
                   QJsonObject json,
                   QString downloadPath);

    class Private;
    std::unique_ptr<Private> d_ptr;
};
