#include "httpclient_p.hpp"

#include <QDebug>
#include <QLocale>

#include <ranges>
#include <utility>

using namespace Qt::StringLiterals;
using Utils::encodeJsonBody;

namespace {

// RFC 9110 token 词法（tchar）：动词等协议元素的合法字符集
bool isTChar(char c)
{
    switch (c) {
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '\'':
    case '*':
    case '+':
    case '-':
    case '.':
    case '^':
    case '_':
    case '`':
    case '|':
    case '~': return true;
    default: return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }
}

bool isValidHttpToken(QByteArrayView token)
{ return !token.isEmpty() && std::ranges::all_of(token, isTChar); }

// HTTP-date（IMF-fixdate）："Sun, 06 Nov 1994 08:49:37 GMT"。
// QLocale::c() 保证英文日/月名（系统 locale 无关），toUTC() 统一为 GMT
QByteArray httpDate(const QDateTime &dateTime)
{ return QLocale::c().toString(dateTime.toUTC(), u"ddd, dd MMM yyyy HH:mm:ss 'GMT'"_s).toLatin1(); }

} // namespace

RequestBuilder::RequestBuilder(HttpClient *client,
                               Kind kind,
                               HttpClient::Method method,
                               QUrl url,
                               std::optional<QByteArray> body,
                               QByteArray contentType,
                               QString downloadPath)
    : d_ptr(std::make_unique<RequestBuilderPrivate>())
{
    d_ptr->clientPtr = client;
    d_ptr->kind = kind;
    d_ptr->method = method;
    d_ptr->url = std::move(url);
    d_ptr->bodyData = std::move(body);
    if (d_ptr->bodyData.has_value() && !contentType.isEmpty()) {
        d_ptr->headers.replaceOrAppend("Content-Type", std::move(contentType));
    }
    d_ptr->downloadPath = std::move(downloadPath);
}

RequestBuilder::RequestBuilder(RequestBuilder &&) noexcept = default;
RequestBuilder &RequestBuilder::operator=(RequestBuilder &&) noexcept = default;
RequestBuilder::~RequestBuilder() = default;

RequestBuilder &RequestBuilder::header(QByteArray name, QByteArray value)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->headers.replaceOrAppend(std::move(name), std::move(value));
    return *this;
}

RequestBuilder &RequestBuilder::bearer(QByteArrayView token)
{ return header("Authorization", "Bearer " + token.toByteArray()); }

RequestBuilder &RequestBuilder::basicAuth(const QString &user, const QString &password)
{
    if (!d_ptr) {
        return *this;
    }
    // client 已销毁时无凭证表可登记：请求头照设，仅跳过挑战-响应回填
    if (d_ptr->clientPtr) {
        d_ptr->clientPtr->registerBasicAuth(d_ptr->url.host(), user, password);
    }
    const auto credentials = u"%1:%2"_s.arg(user, password).toUtf8().toBase64();
    return header("Authorization", "Basic " + credentials);
}

RequestBuilder &RequestBuilder::query(const QString &key, const QString &value)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->queries.append({key, value});
    return *this;
}

// ── 条件请求便利层：各自只是一次 header() 封装，覆盖语义与其一致 ──

RequestBuilder &RequestBuilder::ifNoneMatch(QByteArray etag)
{ return header("If-None-Match", std::move(etag)); }

RequestBuilder &RequestBuilder::ifMatch(QByteArray etag)
{ return header("If-Match", std::move(etag)); }

RequestBuilder &RequestBuilder::ifModifiedSince(const QDateTime &lastModified)
{
    if (!lastModified.isValid()) {
        qCWarning(lcHttpClient)
            << u"RequestBuilder::ifModifiedSince(): invalid QDateTime; header ignored"_s;
        return *this;
    }
    return header("If-Modified-Since", httpDate(lastModified));
}

RequestBuilder &RequestBuilder::ifUnmodifiedSince(const QDateTime &lastModified)
{
    if (!lastModified.isValid()) {
        qCWarning(lcHttpClient)
            << u"RequestBuilder::ifUnmodifiedSince(): invalid QDateTime; header ignored"_s;
        return *this;
    }
    return header("If-Unmodified-Since", httpDate(lastModified));
}

RequestBuilder &RequestBuilder::method(HttpClient::Method method)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->method = method;
    d_ptr->verbOverride.clear();
    return *this;
}

RequestBuilder &RequestBuilder::method(QByteArrayView verb)
{
    if (!d_ptr) {
        return *this;
    }
    if (!isValidHttpToken(verb)) {
        qCWarning(lcHttpClient).noquote() << u"RequestBuilder::method(): invalid HTTP token:"_s
                                          << QByteArray(verb) << u"; ignored"_s;
        return *this;
    }
    // RFC 9110 §9.1：method 大小写敏感，原样保留用户传入的动词
    d_ptr->verbOverride = QByteArray(verb);
    return *this;
}

RequestBuilder &RequestBuilder::body(QByteArray data, QByteArray contentType)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->bodyData = std::move(data);
    if (!contentType.isEmpty()) {
        d_ptr->headers.replaceOrAppend("Content-Type", std::move(contentType));
    }
    return *this;
}

RequestBuilder &RequestBuilder::json(const QJsonObject &obj)
{
    if (!d_ptr) {
        return *this;
    }
    auto encoded = encodeJsonBody(obj);
    return body(std::move(encoded.data), std::move(encoded.contentType));
}

RequestBuilder &RequestBuilder::timeout(std::chrono::milliseconds timeout)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->timeout = timeout;
    return *this;
}

RequestBuilder &RequestBuilder::readBufferLimit(qint64 limit)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->readBufferLimit = limit;
    return *this;
}

RequestBuilder &RequestBuilder::ignoreSslErrors()
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->ignoreSslErrors = true;
    return *this;
}

RequestBuilder &RequestBuilder::clientCert(const QList<QSslCertificate> &certChain,
                                           const QSslKey &key)
{
    if (!d_ptr) {
        return *this;
    }
    const bool hasNullCert
        = std::ranges::any_of(certChain, [](const QSslCertificate &cert) { return cert.isNull(); });
    if (certChain.isEmpty() || hasNullCert || key.isNull()) {
        qCWarning(lcHttpClient).noquote()
            << u"RequestBuilder::clientCert(): empty chain, null certificate, "
               u"or null key; ignored"_s;
        return *this;
    }
    d_ptr->clientCertChain = certChain;
    d_ptr->clientKey = key;
    return *this;
}

RequestBuilder &RequestBuilder::onFinished(HttpFinishedCallback callback)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->onFinished = std::move(callback);
    return *this;
}

RequestBuilder &RequestBuilder::onProgress(HttpProgressCallback callback)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->onProgress = std::move(callback);
    return *this;
}

RequestBuilder &RequestBuilder::putFile(QString filePath)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->payloadKind = PayloadKind::PutFile;
    d_ptr->filePath = std::move(filePath);
    return *this;
}

RequestBuilder &RequestBuilder::putData(QByteArray data)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->payloadKind = PayloadKind::PutData;
    d_ptr->data = std::move(data);
    return *this;
}

RequestBuilder &RequestBuilder::multipartFile(QString filePath)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->payloadKind = PayloadKind::MultipartFile;
    d_ptr->filePath = std::move(filePath);
    return *this;
}

RequestBuilder &RequestBuilder::multipartData(QString fileName, QByteArray data)
{
    if (!d_ptr) {
        return *this;
    }
    d_ptr->payloadKind = PayloadKind::MultipartData;
    d_ptr->fileName = std::move(fileName);
    d_ptr->data = std::move(data);
    return *this;
}

HttpTask RequestBuilder::send()
{
    if (!d_ptr) {
        qCWarning(lcHttpClient)
            << u"RequestBuilder::send(): moved-from builder; request not sent"_s;
        return {};
    }
    if (!d_ptr->clientPtr) {
        qCWarning(lcHttpClient) << u"RequestBuilder::send(): client destroyed; request not sent"_s;
        return {};
    }
    if (std::exchange(d_ptr->consumed, true)) {
        qCWarning(lcHttpClient)
            << u"RequestBuilder::send(): builder already used; request not sent"_s;
        return {};
    }
    return d_ptr->clientPtr->execute(*this);
}

HttpResult RequestBuilder::sync()
{
    if (!d_ptr) {
        qCWarning(lcHttpClient) << u"RequestBuilder::sync(): moved-from builder"_s;
        HttpResult result;
        result.code = HttpErrorCode::InvalidUse;
        result.message = u"Moved-from builder"_s;
        return result;
    }
    if (!d_ptr->clientPtr) {
        qCWarning(lcHttpClient) << u"RequestBuilder::sync(): client destroyed"_s;
        HttpResult result;
        result.code = HttpErrorCode::InvalidUse;
        result.message = u"Client destroyed"_s;
        return result;
    }
    if (std::exchange(d_ptr->consumed, true)) {
        qCWarning(lcHttpClient) << u"RequestBuilder::sync(): builder already used"_s;
        HttpResult result;
        result.code = HttpErrorCode::InvalidUse;
        result.message = u"Builder already used"_s;
        return result;
    }
    if (d_ptr->onFinished) {
        qCWarning(lcHttpClient)
            << u"RequestBuilder::sync(): onFinished ignored; sync() returns the result directly"_s;
    }
    return d_ptr->clientPtr->executeSync(*this);
}
