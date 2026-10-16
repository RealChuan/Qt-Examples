#pragma once

#include <HttpClient/httpclient.hpp>

#include <QDateTime>
#include <QList>
#include <QUrl>

#include <memory>

/**
 * @brief 207 multistatus 中的一个资源（对应一个 <D:response>）
 */
struct DavResource
{
    QUrl href;                 // 已相对请求 URL 解析为绝对，保留百分号编码
    int status = 200;          // 直接 <D:status> 优先，否则取首个 2xx propstat 的状态码
    bool isCollection = false;
    qint64 contentLength = -1; // 缺失/非法为 -1
    QDateTime lastModified;    // HTTP 日期，转 UTC
    QString displayName;
    QString etag;              // 仅取自 2xx propstat
    QString contentType;
    bool failedProps = false;  // 存在非 2xx propstat（属性缺失或未生效）；
                               // PROPFIND 列表常见，不代表资源不存在
};

/**
 * @brief DAV 语义的错误分类（值取对应 HTTP 状态码；NoError/Other 为本地值）
 */
enum class DavErrorCode : int {
    NoError = 0,
    Other = 1,                  // 未映射到 DAV 语义（含网络失败 status==0）
    Multistatus = 207,          // 逐资源状态见 DavResult::resources
    Conflict = 409,             // 目标父集合不存在
    PreconditionFailed = 412,   // Overwrite=F 撞已有目标 / If 前置失败
    UnsupportedMediaType = 415, // 服务器不理解 PROPFIND/LOCK 请求体
    Locked = 423,               // 资源被锁且请求未携带有效令牌
    BadGateway = 502,           // 跨服务器 COPY/MOVE 目的端不可达
    InsufficientStorage = 507,  // 写操作时服务器存储不足
};

/**
 * @brief check() 的产出。message 为英文，本地化按 code 分支而非解析字符串。
 */
struct DavResult
{
    bool success = false;
    QString message;              // 失败原因；成功为空
    DavErrorCode code = DavErrorCode::Other;
    QList<DavResource> resources; // 207 响应的逐资源状态（PROPFIND 载荷 / COPY/MOVE 失败明细）

    Q_REQUIRED_RESULT explicit operator bool() const { return success; }
};

/**
 * @brief WebDAV 客户端：HttpClient 之上的 DAV 词汇层（RFC 4918）
 *
 * WebDAV 是 HTTP 的动词/头/XML 词汇扩展，无新传输行为，故直接继承
 * HttpClient：超时、取消、SSL、认证、进度、断点续传等会话机制继承即用。
 * 词汇工厂只设置动词/头/体默认值，链上后续调用可逐项覆盖（最后一次生效）；
 * 响应侧经 check() 一次调用完成 DAV 语义判定与 207 解析。
 *
 * 锁令牌表：lock() 成功（200）的令牌自动入表（先于用户回调派发）；
 * unlock(url) 自动取表拼 Lock-Token；修改类请求（PUT/DELETE/PROPPATCH/
 * MKCOL/COPY/MOVE）自动携带 If 头，显式设置的 If 优先。表以完全编码
 * URL 为键精确匹配（尾部斜杠敏感）；跨进程恢复等场景经 rememberLock() 补充。
 */
class WebDavClient : public HttpClient
{
    Q_OBJECT

public:
    enum class Depth : int { Zero, One, Infinity };
    Q_ENUM(Depth)

    explicit WebDavClient(QObject *parent = nullptr);
    ~WebDavClient() override;

    // ── 词汇层工厂 ──
    /** @brief PROPFIND（allprop 请求体）；Depth 头按参数。 */
    Q_REQUIRED_RESULT RequestBuilder propfind(QUrl url, Depth depth = Depth::One);
    /** @brief MKCOL；父集合缺失时服务器返回 409。 */
    Q_REQUIRED_RESULT RequestBuilder mkcol(QUrl url);
    /** @brief COPY；destination 须为绝对 URL（RFC 4918），overwrite 映射 Overwrite 头。 */
    Q_REQUIRED_RESULT RequestBuilder copy(QUrl source, QUrl destination, bool overwrite = true);
    /** @brief MOVE；头语义与 copy() 一致。 */
    Q_REQUIRED_RESULT RequestBuilder move(QUrl source, QUrl destination, bool overwrite = true);
    /** @brief OPTIONS：响应头 DAV: 携带 compliance class。 */
    Q_REQUIRED_RESULT RequestBuilder options(QUrl url);
    /** @brief LOCK（exclusive/write lockinfo）；成功令牌自动入表；超时经 header("Timeout", ...) 附加。 */
    Q_REQUIRED_RESULT RequestBuilder lock(QUrl url);
    /** @brief UNLOCK：取表内令牌拼 Lock-Token；无令牌时告警并照常发送。 */
    Q_REQUIRED_RESULT RequestBuilder unlock(QUrl url);
    /** @brief UNLOCK 的显式令牌版本（不经令牌表）。 */
    Q_REQUIRED_RESULT RequestBuilder unlock(QUrl url, QByteArrayView token);

    // ── 锁令牌表 ──
    /** @brief 手动登记令牌（服务器不回显 lockdiscovery、跨进程恢复等场景）。 */
    void rememberLock(const QUrl &url, const QByteArray &token);
    void forgetLock(const QUrl &url);
    [[nodiscard]] bool hasLock(const QUrl &url) const;
    [[nodiscard]] QByteArray lockToken(const QUrl &url) const;

    // ── 响应侧 ──
    /**
     * @brief 对 HttpResult 的一次性 DAV 语义判定：成功/失败、原因、分类，
     *        以及 207 multistatus 的逐资源状态，调用方无需串联其他解析。
     *
     * 判定：2xx 非 207 成功；207 解析后 PROPFIND 成功，其余动词存在非 2xx
     * 资源状态或失败 propstat 即失败（RFC 4918：COPY/MOVE 仅部分失败才回
     * 207；PROPPATCH 恒 207）；
     * 4xx/5xx 取 RFC 4918 §9 措辞（405/409/423 按动词消歧），未命中透传
     * QNAM errorString；本地失败（status==0）透传 result.message。
     */
    Q_REQUIRED_RESULT static DavResult check(const HttpResult &result);

protected:
    /** @brief QNAM 扩展点：修改类请求自动携带已记锁的 If 头（显式 If 优先）。 */
    QNetworkReply *
    createRequest(Operation op, const QNetworkRequest &request, QIODevice *device) override;
    /** @brief HttpClient 扩展点：LOCK/UNLOCK 完成时维护锁令牌表（先于用户回调）。 */
    void resultReady(const HttpResult &result) override;

private:
    class WebDavClientPrivate;
    std::unique_ptr<WebDavClientPrivate> d_ptr;
};
