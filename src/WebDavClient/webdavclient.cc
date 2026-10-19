#include "webdavclient.hpp"

#include <QHash>
#include <QLocale>
#include <QLoggingCategory>
#include <QTimeZone>
#include <QXmlStreamReader>

#include <algorithm>
#include <optional>
#include <utility>

using namespace Qt::StringLiterals;

// 分类日志：默认只输出里程碑与错误；
// 设 QT_LOGGING_RULES="webdavclient.debug=true" 可开启令牌表维护明细
Q_DECLARE_LOGGING_CATEGORY(lcWebDavClient)
Q_LOGGING_CATEGORY(lcWebDavClient, "webdavclient")

namespace {

const QByteArray kPropfindBody
    = QByteArrayLiteral("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                        "<D:propfind xmlns:D=\"DAV:\"><D:allprop/></D:propfind>");

const QByteArray kLockBody = QByteArrayLiteral("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                               "<D:lockinfo xmlns:D=\"DAV:\">"
                                               "<D:lockscope><D:exclusive/></D:lockscope>"
                                               "<D:locktype><D:write/></D:locktype>"
                                               "<D:owner>WebDavClient</D:owner>"
                                               "</D:lockinfo>");

QByteArray depthBytes(WebDavClient::Depth depth)
{
    switch (depth) {
    case WebDavClient::Depth::Zero: return "0";
    case WebDavClient::Depth::One: return "1";
    case WebDavClient::Depth::Infinity: return "infinity";
    }
    Q_UNREACHABLE();
}

// 完全编码形态：与 createRequest() 注入侧同一规范化口径
QString lockKey(const QUrl &url)
{ return url.toString(QUrl::FullyEncoded); }

// DAV 元素匹配：命名空间 URI 严格为 "DAV:"（前缀任意或缺省均可）
bool isDav(const QXmlStreamReader &xml, QStringView name)
{ return xml.name() == name && xml.namespaceUri() == u"DAV:"_s; }

int parseStatusLine(QStringView line)
{
    const auto parts = line.toString().split(u' ', Qt::SkipEmptyParts);
    if (parts.size() < 2) {
        return 0;
    }
    bool ok = false;
    const int code = parts.at(1).toInt(&ok);
    return ok ? code : 0;
}

// RFC 9110 IMF-fixdate："Mon, 13 Oct 2026 08:00:00 GMT"（英文月份由 C locale
// 解析）；非法输入返回无效 QDateTime
QDateTime parseHttpDate(const QString &text)
{
    auto dt = QLocale::c().toDateTime(text, u"ddd, dd MMM yyyy hh:mm:ss 'GMT'"_s);
    return dt.isValid() ? QDateTime{dt.date(), dt.time(), QTimeZone::UTC} : QDateTime{};
}

// 已解析属性位掩码：标准序 propstat 为 (prop, status)——prop 先于 status 到达，
// 属性须先暂存，status 揭晓后再决定并入或丢弃
constexpr int kDisplayName = 1 << 0;
constexpr int kContentLength = 1 << 1;
constexpr int kContentType = 1 << 2;
constexpr int kEtag = 1 << 3;
constexpr int kLastModified = 1 << 4;
constexpr int kResourceType = 1 << 5;

// 每个分支必须完整消费当前元素（readElementText / skipCurrentElement /
// 终止于自身 EndElement 的内层循环），否则 readNextStartElement 会跨层。
// 属性写入 staged 并返回已写字段位掩码
int readProps(QXmlStreamReader &xml, DavResource &staged)
{
    int written = 0;
    while (xml.readNextStartElement()) {
        if (isDav(xml, u"displayname"_s)) {
            staged.displayName = xml.readElementText();
            written |= kDisplayName;
        } else if (isDav(xml, u"getcontentlength"_s)) {
            bool ok = false;
            const auto len = xml.readElementText().trimmed().toLongLong(&ok);
            staged.contentLength = ok ? len : -1;
            written |= kContentLength;
        } else if (isDav(xml, u"getcontenttype"_s)) {
            staged.contentType = xml.readElementText();
            written |= kContentType;
        } else if (isDav(xml, u"getetag"_s)) {
            staged.etag = xml.readElementText().trimmed();
            written |= kEtag;
        } else if (isDav(xml, u"getlastmodified"_s)) {
            staged.lastModified = parseHttpDate(xml.readElementText().trimmed());
            written |= kLastModified;
        } else if (isDav(xml, u"resourcetype"_s)) {
            while (xml.readNextStartElement()) {
                if (isDav(xml, u"collection"_s)) {
                    staged.isCollection = true;
                }
                xml.skipCurrentElement();
            }
            written |= kResourceType;
        } else {
            xml.skipCurrentElement();
        }
    }
    return written;
}

// 掩码内字段并入：多个 2xx propstat 只合并各自实际携带的属性
void applyProps(DavResource &dst, const DavResource &staged, int written)
{
    if (written & kDisplayName) {
        dst.displayName = staged.displayName;
    }
    if (written & kContentLength) {
        dst.contentLength = staged.contentLength;
    }
    if (written & kContentType) {
        dst.contentType = staged.contentType;
    }
    if (written & kEtag) {
        dst.etag = staged.etag;
    }
    if (written & kLastModified) {
        dst.lastModified = staged.lastModified;
    }
    if (written & kResourceType) {
        dst.isCollection = staged.isCollection;
    }
}

// <D:propstat> -> 状态码；RFC 4918 §14.24 子元素序为 (prop, status)：
// 属性先暂存，仅 2xx 时并入，错误 propstat 的属性丢弃
int readPropstat(QXmlStreamReader &xml, DavResource &resource)
{
    int status = 0;
    DavResource staged;
    int written = 0;
    while (xml.readNextStartElement()) {
        if (isDav(xml, u"status"_s)) {
            status = parseStatusLine(xml.readElementText().trimmed());
        } else if (isDav(xml, u"prop"_s)) {
            written |= readProps(xml, staged);
        } else {
            xml.skipCurrentElement();
        }
    }
    if (status >= 200 && status < 300) {
        applyProps(resource, staged, written);
    }
    return status;
}

// 令牌可藏在 prop/lockdiscovery/activelock 任意层级，递归找首个 locktoken/href
std::optional<QByteArray> findLockToken(QXmlStreamReader &xml)
{
    while (xml.readNextStartElement()) {
        if (isDav(xml, u"locktoken"_s)) {
            while (xml.readNextStartElement()) {
                if (isDav(xml, u"href"_s)) {
                    const auto token = xml.readElementText().trimmed().toUtf8();
                    if (!token.isEmpty()) {
                        return token;
                    }
                } else {
                    xml.skipCurrentElement();
                }
            }
            return std::nullopt;
        }
        if (const auto token = findLockToken(xml)) {
            return token;
        }
    }
    return std::nullopt;
}

// If 头注入候选：PUT/DELETE 枚举操作 + DAV 修改动词；LOCK/UNLOCK 各有
// 令牌头，GET/PROPFIND/OPTIONS 只读。CustomVerbAttribute 由
// sendCustomRequest 写入；动词两侧均已大写，精确匹配
bool isReadOnlyOperation(QNetworkAccessManager::Operation op, const QNetworkRequest &request)
{
    switch (op) {
    case QNetworkAccessManager::PutOperation:
    case QNetworkAccessManager::DeleteOperation: return false;
    case QNetworkAccessManager::CustomOperation: {
        const auto verb = request.attribute(QNetworkRequest::CustomVerbAttribute).toString();
        return !(verb == u"PUT"_s || verb == u"DELETE"_s || verb == u"PROPPATCH"_s
                 || verb == u"MKCOL"_s || verb == u"COPY"_s || verb == u"MOVE"_s);
    }
    default: return true;
    }
}

// RFC 4918 §9 状态措辞（405/409/423 按动词消歧）；未命中透传 result.message
//（QNAM errorString 已含 "server replied: <reason>"）
QString davMessage(const HttpResult &result)
{
    switch (result.status) {
    case 403: return u"Forbidden: no permission for this operation"_s;
    case 405:
        if (result.method == "MKCOL") {
            return u"Collection already exists"_s;
        }
        return u"Method not allowed"_s;
    case 409:
        if (result.method == "MKCOL" || result.method == "COPY" || result.method == "MOVE") {
            return u"Parent collection of the target does not exist"_s;
        }
        return u"Conflict"_s;
    case 412:
        return u"Precondition failed: destination exists with Overwrite=F, or the If "
               u"header was rejected"_s;
    case 415: return u"The server does not support the request body media type"_s;
    case 423:
        if (result.method == "LOCK") {
            return u"A conflicting lock exists on the resource"_s;
        }
        return u"The resource is locked (acquire a token via lock())"_s;
    case 502: return u"The destination server is unreachable (cross-server COPY/MOVE)"_s;
    case 507: return u"Insufficient storage"_s;
    default: return result.message;
    }
}

DavErrorCode classify(const HttpResult &result)
{
    switch (result.status) {
    case 207: return DavErrorCode::Multistatus;
    case 409: return DavErrorCode::Conflict;
    case 412: return DavErrorCode::PreconditionFailed;
    case 415: return DavErrorCode::UnsupportedMediaType;
    case 423: return DavErrorCode::Locked;
    case 502: return DavErrorCode::BadGateway;
    case 507: return DavErrorCode::InsufficientStorage;
    default: break;
    }
    if (result.success()) {
        return DavErrorCode::NoError;
    }
    return DavErrorCode::Other;
}

// 解析 207 multistatus；XML 非法或某 <D:response> 缺 href 时返回 nullopt
//（不产部分结果）
std::optional<QList<DavResource>> parseMultistatus(const HttpResult &result)
{
    QList<DavResource> resources;
    QXmlStreamReader xml(result.body);
    bool foundRoot = false;
    while (xml.readNextStartElement()) {
        if (isDav(xml, u"multistatus"_s)) {
            foundRoot = true;
            break;
        }
        xml.skipCurrentElement();
    }
    if (!foundRoot) {
        return std::nullopt;
    }

    while (xml.readNextStartElement()) {
        if (!isDav(xml, u"response"_s)) {
            xml.skipCurrentElement();
            continue;
        }
        DavResource resource;
        bool hasHref = false;
        int directStatus = 0;
        int propsStatus = 0;
        while (xml.readNextStartElement()) {
            if (isDav(xml, u"href"_s)) {
                resource.href = result.url.resolved(QUrl(xml.readElementText().trimmed()));
                hasHref = true;
            } else if (isDav(xml, u"status"_s)) {
                directStatus = parseStatusLine(xml.readElementText().trimmed());
            } else if (isDav(xml, u"propstat"_s)) {
                const int status = readPropstat(xml, resource);
                if (status >= 200 && status < 300) {
                    if (propsStatus == 0) {
                        propsStatus = status;
                    }
                } else if (status != 0) {
                    resource.failedProps = true;
                }
            } else {
                xml.skipCurrentElement();
            }
        }
        if (!hasHref) {
            return std::nullopt; // 缺 href 属契约违反：整体失败优于部分结果
        }
        resource.status = directStatus != 0 ? directStatus : (propsStatus != 0 ? propsStatus : 200);
        resources.append(std::move(resource));
    }

    if (xml.hasError()) {
        return std::nullopt;
    }
    return resources;
}

std::optional<QByteArray> parseLockToken(const QByteArray &body)
{
    QXmlStreamReader xml(body);
    return findLockToken(xml);
}

// RFC 4918 If 头列表语法的单令牌形式
QByteArray ifHeader(QByteArrayView token)
{ return "(<" + token.toByteArray() + ">)"; }

} // namespace

class WebDavClient::WebDavClientPrivate
{
public:
    explicit WebDavClientPrivate(WebDavClient *q) : q_ptr(q) {}

    // 值不含尖括号（拼 If/Lock-Token 头时补）
    QHash<QString, QByteArray> lockTokens;

    WebDavClient *q_ptr;
};

WebDavClient::WebDavClient(QObject *parent)
    : HttpClient(parent), d_ptr(std::make_unique<WebDavClientPrivate>(this))
{}

WebDavClient::~WebDavClient() = default;

// 工厂采用具名构建器：链式方法返回左值引用，直接 return 链式表达式会请求
// 已删除的拷贝构造（RequestBuilder 仅可移动）

RequestBuilder WebDavClient::propfind(QUrl url, Depth depth)
{
    auto builder = get(std::move(url));
    builder.method("PROPFIND")
        .header("Depth", depthBytes(depth))
        .body(kPropfindBody, "application/xml; charset=utf-8");
    return builder;
}

RequestBuilder WebDavClient::mkcol(QUrl url)
{
    auto builder = get(std::move(url));
    builder.method("MKCOL");
    return builder;
}

RequestBuilder WebDavClient::copy(QUrl source, QUrl destination, bool overwrite)
{
    if (destination.isRelative()) {
        qCWarning(lcWebDavClient).noquote()
            << "copy(): Destination must be an absolute URL per RFC 4918:" << destination;
    }
    auto builder = get(std::move(source));
    builder.method("COPY")
        .header("Destination", destination.toString(QUrl::FullyEncoded).toUtf8())
        .header("Overwrite", overwrite ? "T" : "F");
    return builder;
}

RequestBuilder WebDavClient::move(QUrl source, QUrl destination, bool overwrite)
{
    if (destination.isRelative()) {
        qCWarning(lcWebDavClient).noquote()
            << "move(): Destination must be an absolute URL per RFC 4918:" << destination;
    }
    auto builder = get(std::move(source));
    builder.method("MOVE")
        .header("Destination", destination.toString(QUrl::FullyEncoded).toUtf8())
        .header("Overwrite", overwrite ? "T" : "F");
    return builder;
}

RequestBuilder WebDavClient::options(QUrl url)
{
    auto builder = get(std::move(url));
    builder.method("OPTIONS");
    return builder;
}

RequestBuilder WebDavClient::lock(QUrl url)
{
    auto builder = get(std::move(url));
    builder.method("LOCK").body(kLockBody, "application/xml; charset=utf-8");
    return builder;
}

RequestBuilder WebDavClient::unlock(QUrl url)
{
    const auto token = lockToken(url);
    if (token.isEmpty()) {
        qCWarning(lcWebDavClient).noquote()
            << "unlock(): no remembered lock token for" << url << "; sending without Lock-Token";
    }
    auto builder = get(std::move(url));
    builder.method("UNLOCK");
    if (!token.isEmpty()) {
        builder.header("Lock-Token", "<" + token + ">");
    }
    return builder;
}

RequestBuilder WebDavClient::unlock(QUrl url, QByteArrayView token)
{
    auto builder = get(std::move(url));
    builder.method("UNLOCK");
    if (!token.isEmpty()) {
        builder.header("Lock-Token", "<" + token.toByteArray() + ">");
    }
    return builder;
}

void WebDavClient::rememberLock(const QUrl &url, const QByteArray &token)
{ d_ptr->lockTokens.insert(lockKey(url), token); }

void WebDavClient::forgetLock(const QUrl &url)
{ d_ptr->lockTokens.remove(lockKey(url)); }

bool WebDavClient::hasLock(const QUrl &url) const
{ return d_ptr->lockTokens.contains(lockKey(url)); }

QByteArray WebDavClient::lockToken(const QUrl &url) const
{ return d_ptr->lockTokens.value(lockKey(url)); }

DavResult WebDavClient::check(const HttpResult &result)
{
    if (result.success()) {
        if (result.status != 207) {
            return {.success = true, .code = DavErrorCode::NoError};
        }
        const auto parsed = parseMultistatus(result);
        if (!parsed) {
            return {.success = false,
                    .message = u"Malformed 207 multistatus body"_s,
                    .code = DavErrorCode::Multistatus};
        }
        if (result.method == "PROPFIND") {
            return {.success = true, .code = DavErrorCode::Multistatus, .resources = *parsed};
        }
        // COPY/MOVE 仅部分失败才回 207；PROPPATCH 恒 207：按逐资源状态与
        // propstat 结果判定
        const auto failed = std::ranges::count_if(*parsed, [](const DavResource &r) {
            return r.status < 200 || r.status >= 300 || r.failedProps;
        });
        if (failed == 0) {
            return {.success = true, .code = DavErrorCode::Multistatus, .resources = *parsed};
        }
        return {.success = false,
                .message
                = u"Partial failure: %1 of %2 resources failed"_s.arg(failed).arg(parsed->size()),
                .code = DavErrorCode::Multistatus,
                .resources = *parsed};
    }

    if (result.status == 0) {
        return {.success = false, .message = result.message, .code = DavErrorCode::Other};
    }
    return {.success = false, .message = davMessage(result), .code = classify(result)};
}

QNetworkReply *
WebDavClient::createRequest(Operation op, const QNetworkRequest &request, QIODevice *device)
{
    QNetworkRequest req = request;
    if (!isReadOnlyOperation(op, req)) {
        const auto it = d_ptr->lockTokens.constFind(lockKey(req.url()));
        if (it != d_ptr->lockTokens.constEnd() && !req.hasRawHeader("If")) {
            req.setRawHeader("If", ifHeader(*it));
            qCDebug(lcWebDavClient).noquote()
                << "auto-attaching If lock token to" << req.url().toString();
        }
    }
    return QNetworkAccessManager::createRequest(op, req, device);
}

void WebDavClient::resultReady(const HttpResult &result)
{
    if (result.method != "LOCK"_ba && result.method != "UNLOCK"_ba) {
        return;
    }
    // 键口径：result.url 为 reply->url()（重定向跟随后的地址）。LOCK/UNLOCK
    // 在 Qt 默认重定向策略下不自动跟随，与 createRequest() 注入侧一致
    const auto key = lockKey(result.url);
    if (result.method == "LOCK"_ba && result.status == 200) {
        if (const auto token = parseLockToken(result.body)) {
            d_ptr->lockTokens.insert(key, *token);
            qCDebug(lcWebDavClient).noquote() << "lock token remembered for" << key;
        }
    } else if (result.method == "UNLOCK"_ba && (result.status == 204 || result.status == 200)) {
        d_ptr->lockTokens.remove(key);
        qCDebug(lcWebDavClient).noquote() << "lock token released for" << key;
    }
}
