#include "httpclient_p.hpp"

#include <QDebug>
#include <QMetaEnum>
#include <QSslConfiguration>
#include <QUrlQuery>

#include <array>
#include <ranges>
#include <utility>

using namespace Qt::StringLiterals;
using Utils::TaskState;

namespace {

// 落盘块：1 MiB。readyRead 槽内同步写，过大会拉长事件循环阻塞，
// 1 MiB 已摊薄 write() 次数，再大收益递减
constexpr qint64 kDownloadChunkSize = 1024 * 1024;

// 凭证类头部只打印占位，避免 token/会话泄漏到日志。
// 头名为 ASCII：折叠一次后在字节域匹配（QByteArrayView 无大小写不敏感查找），
// 免去每次调用一次 QString 分配与 Latin-1 到 UTF-16 的转换
bool isSensitiveHeader(QByteArrayView name)
{
    static const std::array sensitiveSubstrings{"authorization"_ba,
                                                "cookie"_ba,
                                                "token"_ba,
                                                "secret"_ba,
                                                "api_key"_ba,
                                                "apikey"_ba,
                                                "api-key"_ba};
    const QByteArray lowered = name.toByteArray().toLower();
    return std::ranges::any_of(sensitiveSubstrings, [&lowered](QByteArrayView substring) {
        return lowered.contains(substring);
    });
}

// 本地错误码的可读消息；网络错误不经过此函数，消息直接取 QNetworkReply::errorString()
QString httpErrorMessage(HttpErrorCode code)
{
    switch (code) {
    case HttpErrorCode::Timeout: return u"Request timed out"_s;
    case HttpErrorCode::FileError: return u"File operation failed"_s;
    default: return {};
    }
}

} // namespace

namespace Utils {

QByteArray redactedHeader(QByteArrayView name, const QByteArray &value)
{ return isSensitiveHeader(name) ? QByteArray("***") : value; }

BodyEncoding encodeJsonBody(const QJsonObject &json)
{
    return {QJsonDocument(json).toJson(QJsonDocument::Compact),
            QByteArrayLiteral("application/json; charset=utf-8")};
}

} // namespace Utils

QByteArray HttpClient::HttpClientPrivate::methodBytes(HttpClient::Method method)
{
    // Q_ENUM 反射取得枚举名（"Get"），统一大写化为 HTTP 动词（"GET"）
    return QByteArray(
               QMetaEnum::fromType<HttpClient::Method>().valueToKey(static_cast<int>(method)))
        .toUpper();
}

// 动词决议：字符串覆盖（method(QByteArrayView)）优先于枚举，
// 与 method() 两个重载"最后一次调用生效"的契约一致
QByteArray
HttpClient::HttpClientPrivate::effectiveVerb(const RequestBuilder::RequestBuilderPrivate &config)
{ return config.verbOverride.isEmpty() ? methodBytes(config.method) : config.verbOverride; }

HttpResult HttpClient::HttpClientPrivate::fileError(const QUrl &url,
                                                    const QString &path,
                                                    const QString &reason)
{
    HttpResult result;
    result.code = HttpErrorCode::FileError;
    result.url = url;
    result.message = u"Cannot open file %1: %2"_s.arg(path, reason);
    qCWarning(lcHttpClient).noquote() << result.message;
    return result;
}

HttpResult HttpClient::HttpClientPrivate::invalidUse(const QUrl &url, const QString &reason)
{
    HttpResult result;
    result.code = HttpErrorCode::InvalidUse;
    result.url = url;
    result.message = reason;
    qCWarning(lcHttpClient).noquote() << result.message;
    return result;
}

QNetworkRequest
HttpClient::HttpClientPrivate::buildRequest(const RequestBuilder::RequestBuilderPrivate &config,
                                            QByteArrayView excludeHeader)
{
    QNetworkRequest request;
    auto url = config.url;
    if (!config.queries.isEmpty()) {
        QUrlQuery query(url);
        for (const auto &[key, value] : std::as_const(config.queries)) {
            query.addQueryItem(key, value);
        }
        url.setQuery(query);
    }
    request.setUrl(url);
    // 隐式共享拷贝；excludeHeader（如 download 的 Range）由调用方独占管理，用户值移除
    auto headers = config.headers;
    if (!excludeHeader.isEmpty()) {
        const auto excluded = QByteArray{excludeHeader};
        if (headers.contains(excluded)) {
            qCDebug(lcHttpClient).noquote()
                << "header" << excluded << "is managed exclusively; user value ignored";
            headers.removeAll(excluded);
        }
    }
    for (const auto &[name, value] : headers.toListOfPairs()) {
        qCDebug(lcHttpClient).noquote()
            << "request header:" << name << Utils::redactedHeader(name, value);
    }
    request.setHeaders(std::move(headers));
    if (!config.clientCertChain.isEmpty()) {
        // 在请求当前 SSL 配置之上叠加证书链与私钥，自定义 CA 等既有字段保留
        auto sslConfiguration = request.sslConfiguration();
        sslConfiguration.setLocalCertificateChain(config.clientCertChain);
        sslConfiguration.setPrivateKey(config.clientKey);
        request.setSslConfiguration(sslConfiguration);
    }
    return request;
}

QString
HttpClient::HttpClientPrivate::drainToFile(QNetworkReply *reply, QFile *file, QByteArray &chunk)
{
    if (chunk.size() != int(kDownloadChunkSize)) {
        chunk.resize(int(kDownloadChunkSize));
    }
    while (reply->bytesAvailable() > 0) {
        const qint64 read = reply->read(chunk.data(), chunk.size());
        if (read <= 0) {
            break; // 暂无数据可读：readyRead 与实际到达竞态时的正常退出
        }
        if (file->write(chunk.constData(), read) != read) {
            auto error = file->errorString();
            if (error.isEmpty()) {
                error = u"write failed"_s;
            }
            return error;
        }
    }
    return {};
}

HttpResult HttpClient::HttpClientPrivate::buildResult(QNetworkReply *reply, const TaskState &state)
{
    HttpResult result;
    result.url = reply->url();
    result.method = state.method;
    result.status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    result.headers = reply->headers();
    result.sslErrors = state.sslErrors;
    const auto error = reply->error();
    if (error == QNetworkReply::NoError) {
        result.code = HttpErrorCode::NoError;
    } else if (error == QNetworkReply::OperationCanceledError) {
        // 用户取消路径会先 disconnect 再 abort，不会进入槽。到达此处的
        // OperationCanceledError 有两个来源，须按 timedOut 归因：
        //  - 超时定时器的 abort -> Timeout
        //  - QNAM 内部取消（HTTP/2 RST_STREAM、代理取消等）-> Canceled
        if (state.timedOut) {
            result.code = HttpErrorCode::Timeout;
            result.message = httpErrorMessage(HttpErrorCode::Timeout);
        } else {
            result.code = HttpErrorCode::Canceled;
            result.message = reply->errorString();
        }
    } else {
        // 网络层失败与服务器错误状态（QNAM 映射码），消息直接用 errorString()
        result.code = static_cast<HttpErrorCode>(error);
        result.message = reply->errorString();
    }
    return result;
}
