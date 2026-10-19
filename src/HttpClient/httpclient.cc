#include "httpclient.hpp"

#include <QAuthenticator>
#include <QEventLoop>
#include <QFileInfo>
#include <QHash>
#include <QHttpMultiPart>
#include <QLocale>
#include <QLoggingCategory>
#include <QPointer>
#include <QSslConfiguration>
#include <QStringView>
#include <QTimer>
#include <QUrlQuery>

#include <algorithm>
#include <array>
#include <ranges>
#include <unordered_map>
#include <utility>

using namespace Qt::StringLiterals;

// 分类日志：默认只输出里程碑（info）与错误（warning）；
// 设 QT_LOGGING_RULES="httpclient.debug=true" 可开启请求/响应明细，便于调试
Q_DECLARE_LOGGING_CATEGORY(lcHttpClient)
Q_LOGGING_CATEGORY(lcHttpClient, "httpclient")

namespace {

// 下载内存防护：QNetworkReply 读缓冲默认无上限，高速下载 + 落盘慢时无界堆积，
// 32 位地址空间下必然 OOM。两层防护：readBufferSize() 限制在途窗口实现背压，
// drainToFile() 分块落盘避免 readAll() 的整体分配复制。窗口取 MB 级而非流式
// 消费的 KB 级：Qt 为跨线程异步投递，需覆盖"带宽 × 反馈延迟"再留余量。

// 按指针宽度判定（Q_PROCESSOR_WORDSIZE 识别不了 64 位 CPU 上的 32 位程序）
constexpr bool kIs32BitProcess = sizeof(void *) == 4;

// 在途窗口：32 位 4 MiB（地址空间紧），64 位 8 MiB；readBufferLimit() 可按请求覆盖
constexpr qint64 kDownloadReadBufferLimit = kIs32BitProcess ? 4 * 1024 * 1024 : 8 * 1024 * 1024;

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

// 敏感头值打码（与 redactedUrl 的 *** 占位约定一致）
QByteArray redactedHeader(QByteArrayView name, const QByteArray &value)
{ return isSensitiveHeader(name) ? QByteArray("***") : value; }

// query 中的凭证类键（预签名 URL、OAuth token 等）：子串匹配覆盖键名变体
//（access_token、X-Amz-Signature 等），X-Amz-Date 等非凭证参数不命中
bool isSensitiveQueryKey(const QString &key)
{
    static const std::array<QStringView, 8> sensitiveSubstrings{
        u"token",
        u"secret",
        u"signature",
        u"credential",
        u"password",
        u"passwd",
        u"api_key",
        u"apikey",
    };
    return std::ranges::any_of(sensitiveSubstrings, [&key](const auto &substring) {
        return key.contains(substring, Qt::CaseInsensitive);
    });
}

// URL 入日志的唯一通道：userinfo 密码与 query 凭证值替换为占位符（与敏感头
// 的 *** 约定一致），用户名与非敏感参数保留。qCInfo/warning 默认输出、
// 不受 debug 开关控制，故必须在输出前脱敏
QString redactedUrl(const QUrl &url)
{
    const bool hasPassword = !url.password().isEmpty();
    bool hasSensitiveQuery = false;
    if (url.hasQuery()) {
        const QUrlQuery query(url);
        hasSensitiveQuery = std::ranges::any_of(
            query.queryItems(), [](const auto &item) { return isSensitiveQueryKey(item.first); });
    }
    if (!hasPassword && !hasSensitiveQuery) {
        return url.toString();
    }
    auto copy = url;
    if (hasPassword) {
        copy.setPassword(u"***"_s);
    }
    if (hasSensitiveQuery) {
        QUrlQuery query(copy);
        auto items = query.queryItems();
        for (auto &[key, value] : items) {
            if (isSensitiveQueryKey(key)) {
                value = u"***"_s;
            }
        }
        query.setQueryItems(items);
        copy.setQuery(query);
    }
    return copy.toString();
}

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

// multipart filename 编码：RFC 5987 attr-char 白名单之外的字节（含 " \ CR LF、
// 空格与非 ASCII）全部 %XX——quoted-string 转义、part 头注入、文件名编码三件
// 事一次解决；服务端（PHP/Werkzeug/multer 等）普遍按 percent-encoded 解读
QByteArray encodeMultipartFilename(const QString &fileName)
{
    static const char attrChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789"
                                    "!#$&+-.^_`|~";
    return fileName.toUtf8().toPercentEncoding(attrChars);
}

// JSON 序列化为请求体：空对象产出 "{}"（有效 JSON），
// 与 std::nullopt 表达的"无 body"严格区分
struct BodyEncoding
{
    QByteArray data;
    QByteArray contentType;
};

BodyEncoding encodeJsonBody(const QJsonObject &json)
{
    return {QJsonDocument(json).toJson(QJsonDocument::Compact),
            QByteArrayLiteral("application/json; charset=utf-8")};
}

// HTTP-date（IMF-fixdate）："Sun, 06 Nov 1994 08:49:37 GMT"。
// QLocale::c() 保证英文日/月名（系统 locale 无关），toUTC() 统一为 GMT
QByteArray httpDate(const QDateTime &dateTime)
{ return QLocale::c().toString(dateTime.toUTC(), u"ddd, dd MMM yyyy HH:mm:ss 'GMT'"_s).toLatin1(); }

// 丢弃下载 .temp：先回退本次响应的写入（downloadBase 为起点）再删——删除
// 失败（Windows 上杀毒/索引服务常短暂占用刚关闭的文件）时残留的也是干净
// 断点，不会被下次续传拼坏、也不会随重试膨胀；清理失败不改变请求结果，仅告警
void discardTempFile(const QString &tempPath, qint64 downloadBase)
{
    QFile::resize(tempPath, downloadBase);
    QFile temp(tempPath);
    if (!temp.remove()) {
        qCWarning(lcHttpClient).noquote()
            << "cannot remove temp file" << tempPath << ":" << temp.errorString();
    }
}

} // namespace

// 本地错误码的可读消息；网络错误不经过此函数，消息直接取 QNetworkReply::errorString()
static QString httpErrorMessage(HttpErrorCode code)
{
    switch (code) {
    case HttpErrorCode::Timeout: return u"Request timed out"_s;
    case HttpErrorCode::FileError: return u"File operation failed"_s;
    default: return {};
    }
}

QList<QByteArray> HttpResult::header(QByteArrayView name) const
{ return headers.values(name); }

class RequestBuilder::RequestBuilderPrivate
{
public:
    RequestBuilderPrivate() = default;

    HttpClient *client = nullptr;
    RequestBuilder::Kind kind = RequestBuilder::Kind::Json;
    HttpClient::Method method = HttpClient::Method::Get;

    // body 三态：nullopt = 未设置；有值 = 已设置（空字节即空体；
    // json({}) 序列化为 "{}"，与"无 body"严格区分）
    std::optional<QByteArray> bodyData;

    QByteArray verbOverride; // 字符串动词覆盖；effectiveVerb() 决议

    QUrl url;
    QString downloadPath;

    RequestBuilder::PayloadKind payloadKind = RequestBuilder::PayloadKind::None;
    QString filePath;
    QByteArray data;
    QString fileName;

    QHttpHeaders headers; // 覆盖语义由 replaceOrAppend 提供（同名大小写不敏感替换）
    QList<QPair<QString, QString>> queries;
    std::chrono::milliseconds timeout{0};
    bool ignoreSslErrors = false;
    QList<QSslCertificate> clientCertChain; // mTLS 客户端证书链（空 = 未设置，链首为叶证书）
    QSslKey clientKey;
    qint64 readBufferLimit = 0; // 下载在途窗口覆盖值；0 = 使用默认 kDownloadReadBufferLimit
    bool consumed = false;      // send()/sync() 一次性守卫
    HttpFinishedCallback onFinished;
    HttpProgressCallback onProgress;
};

// 取消句柄的内部状态：QPointer 弱引用，不延长任何生命周期
class HttpTask::HttpTaskPrivate
{
public:
    HttpTaskPrivate() = default;

    QPointer<HttpClient> client;
    QPointer<QNetworkReply> reply;
};

// 每个在途请求的执行期状态，由 tasks map 以 unique_ptr 持有（RAII）：
// 析构即停止超时定时器、关闭文件句柄
class HttpClient::HttpClientPrivate
{
public:
    explicit HttpClientPrivate(HttpClient *q) : q_ptr(q) {}

    struct TaskState
    {
        HttpFinishedCallback onFinished;
        HttpProgressCallback onProgress;
        std::unique_ptr<QFile> file;          // 上传：源文件；下载：目标 .temp
        std::unique_ptr<QTimer> timeoutTimer; // 总时长超时定时器
        QString downloadPath;                 // 仅下载：最终路径
        qint64 downloadBase = 0;              // 仅下载：断点续传偏移
        bool ignoreSsl = false;
        bool rangeChecked = false;            // 仅下载：首个 readyRead 已处理 200 截断分支
        QString fileError;                    // 仅下载：写入/保存失败原因；非空即失败
        // QNetworkReply 无 sslErrors() getter，信号是唯一来源：槽中收集，buildResult 时带入
        QList<QSslError> sslErrors;
        QByteArray method;     // 请求动词（QNetworkReply 无对应 getter）
        bool timedOut = false; // 超时定时器已触发：buildResult 据此区分 Timeout 与 QNAM 内部取消
        QByteArray ioChunk;    // drainToFile 复用的落盘缓冲：跨多次 readyRead 避免重复分配 1 MiB
    };

    // prepare 的产出：reply 为空表示请求未发起（文件打开失败等），error 携带原因
    struct Prepared
    {
        QNetworkReply *reply = nullptr;
        std::unique_ptr<TaskState> state;
        HttpResult error;

        [[nodiscard]] bool ok() const { return reply != nullptr; }

        static Prepared fail(HttpResult err, std::unique_ptr<TaskState> state = nullptr)
        { return {.state = std::move(state), .error = std::move(err)}; }
    };

    static QByteArray methodBytes(HttpClient::Method method)
    {
        // Q_ENUM 反射取得枚举名（"Get"），统一大写化为 HTTP 动词（"GET"）
        return QByteArray(
                   QMetaEnum::fromType<HttpClient::Method>().valueToKey(static_cast<int>(method)))
            .toUpper();
    }

    // 动词决议：字符串覆盖（method(QByteArrayView)）优先于枚举，
    // 与 method() 两个重载"最后一次调用生效"的契约一致
    static QByteArray effectiveVerb(const RequestBuilder::RequestBuilderPrivate &config)
    { return config.verbOverride.isEmpty() ? methodBytes(config.method) : config.verbOverride; }

    static HttpResult fileError(const QUrl &url, const QString &path, const QString &reason)
    {
        HttpResult result;
        result.code = HttpErrorCode::FileError;
        result.url = url;
        result.message = u"Cannot open file %1: %2"_s.arg(path, reason);
        qCWarning(lcHttpClient).noquote() << result.message;
        return result;
    }

    static HttpResult invalidUse(const QUrl &url, const QString &reason)
    {
        HttpResult result;
        result.code = HttpErrorCode::InvalidUse;
        result.url = url;
        result.message = reason;
        qCWarning(lcHttpClient).noquote() << result.message;
        return result;
    }

    // excludeHeader：由调用方独占管理的头（大小写不敏感）。用户经 header()
    // 设置的该头不进入最终请求（如 download() 的 Range），避免双重管理
    QNetworkRequest buildRequest(const RequestBuilder::RequestBuilderPrivate &config,
                                 QByteArrayView excludeHeader = {})
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
                << "request header:" << name << redactedHeader(name, value);
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

    std::unique_ptr<TaskState> buildTaskState(const RequestBuilder::RequestBuilderPrivate &config)
    {
        auto state = std::make_unique<TaskState>();
        state->method = effectiveVerb(config);
        state->onFinished = config.onFinished;
        state->onProgress = config.onProgress;
        state->ignoreSsl = config.ignoreSslErrors;
        if (config.timeout > std::chrono::milliseconds{0}) {
            state->timeoutTimer = std::make_unique<QTimer>();
            state->timeoutTimer->setSingleShot(true);
            q_ptr->connect(
                state->timeoutTimer.get(), &QTimer::timeout, q_ptr, &HttpClient::onTimeout);
            state->timeoutTimer->start(config.timeout);
        }
        return state;
    }

    // 分块排空 reply 并写入下载文件（readAll() 整体分配复制在 32 位下会 OOM）；
    // chunk 由 TaskState 持有、跨多次 readyRead 复用。返回空串表示成功，
    // 非空为写入失败原因（磁盘满等）
    static QString drainToFile(QNetworkReply *reply, QFile *file, QByteArray &chunk)
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

    QNetworkReply *postMultipart(const QNetworkRequest &request,
                                 const QString &fileName,
                                 QIODevice *device,
                                 const QByteArray &data)
    {
        QHttpPart filePart;
        // 整值以 QByteArray 组装走 setRawHeader，不经 QVariant 的字符串隐式转换；
        // filename 的转义/防注入/非 ASCII 编码见 encodeMultipartFilename
        filePart.setRawHeader("Content-Disposition",
                              "form-data; name=\"file\"; filename=\""_ba
                                  + encodeMultipartFilename(fileName) + '"');
        if (device) {
            filePart.setBodyDevice(device);
        } else {
            filePart.setBody(data);
        }
        // QHttpMultiPart 归 reply 所有（Qt 对象树），随 reply 析构自动释放
        auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        multiPart->append(filePart);
        auto *reply = q_ptr->post(request, multiPart);
        multiPart->setParent(reply);
        return reply;
    }

    void connectSignals(const RequestBuilder::RequestBuilderPrivate &config,
                        QNetworkReply *reply,
                        TaskState *state)
    {
        q_ptr->connect(reply, &QNetworkReply::sslErrors, q_ptr, &HttpClient::onSslErrors);
        switch (config.kind) {
        case RequestBuilder::Kind::Json:
        case RequestBuilder::Kind::Upload:
            q_ptr->connect(reply, &QNetworkReply::finished, q_ptr, &HttpClient::onReplyFinished);
            if (state->onProgress) {
                q_ptr->connect(
                    reply, &QNetworkReply::uploadProgress, q_ptr, &HttpClient::onUploadProgress);
            }
            break;
        case RequestBuilder::Kind::Download:
            q_ptr->connect(reply, &QNetworkReply::finished, q_ptr, &HttpClient::onDownloadFinished);
            q_ptr->connect(
                reply, &QNetworkReply::readyRead, q_ptr, &HttpClient::onDownloadReadyRead);
            // 重定向会重建 HTTP 线程 delegate（窗口归零），每个响应到达时重设背压
            const auto readBufferLimit
                = config.readBufferLimit > 0 ? config.readBufferLimit : kDownloadReadBufferLimit;
            q_ptr->connect(reply, &QNetworkReply::metaDataChanged, reply, [reply, readBufferLimit] {
                reply->setReadBufferSize(readBufferLimit);
            });
            if (state->onProgress) {
                q_ptr->connect(reply,
                               &QNetworkReply::downloadProgress,
                               q_ptr,
                               &HttpClient::onDownloadProgress);
            }
            break;
        }
    }

    Prepared prepare(const RequestBuilder::RequestBuilderPrivate &config)
    {
        Prepared prepared;
        prepared.state = buildTaskState(config);
        auto &state = prepared.state;

        const auto verb = effectiveVerb(config);

        switch (config.kind) {
        case RequestBuilder::Kind::Json: {
            auto request = buildRequest(config);
            QByteArray body;
            if (config.bodyData.has_value()) {
                body = *config.bodyData;
                if (!body.isEmpty()) {
                    qCDebug(lcHttpClient).noquote() << "request body:" << body;
                }
            }
            prepared.reply = q_ptr->sendCustomRequest(request, verb, body);
            break;
        }
        case RequestBuilder::Kind::Download: {
            const auto tempPath = config.downloadPath + u".temp"_s;
            auto file = std::make_unique<QFile>(tempPath);
            if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
                return Prepared::fail(fileError(config.url, tempPath, file->errorString()),
                                      std::move(prepared.state));
            }
            auto request = buildRequest(config, "Range");
            state->downloadBase = file->size();
            if (state->downloadBase > 0) {
                request.setRawHeader(
                    "Range", "bytes="_ba + QByteArray::number(state->downloadBase) + "-"_ba);
            }
            // 纯 GET 走原生 get() 快路径；改动词或带动作体经自定义动词发送
            const bool needCustomVerb = config.bodyData.has_value()
                                        || !config.verbOverride.isEmpty()
                                        || config.method != HttpClient::Method::Get;
            if (needCustomVerb) {
                QByteArray body;
                if (config.bodyData.has_value()) {
                    body = *config.bodyData;
                    if (!body.isEmpty()) {
                        qCDebug(lcHttpClient).noquote() << "request body:" << body;
                    }
                }
                prepared.reply = q_ptr->sendCustomRequest(request, verb, body);
            } else {
                prepared.reply = q_ptr->get(request);
            }
            // 背压窗口需排队设置：setReadBufferSize() 经信号通知 HTTP 线程 delegate，
            // 而信号连接在请求启动（_q_startOperation，get() 后的排队事件）才建立，
            // 同步调用会因信号无接收者而失效
            const auto readBufferLimit
                = config.readBufferLimit > 0 ? config.readBufferLimit : kDownloadReadBufferLimit;
            QMetaObject::invokeMethod(
                prepared.reply,
                [reply = prepared.reply, readBufferLimit] {
                    reply->setReadBufferSize(readBufferLimit);
                },
                Qt::QueuedConnection);
            state->file = std::move(file);
            state->downloadPath = config.downloadPath;
            break;
        }
        case RequestBuilder::Kind::Upload: {
            if (config.bodyData.has_value()) {
                return Prepared::fail(
                    invalidUse(config.url,
                               u"upload() payload is provided by putFile/putData/multipart*; "
                               u"body()/json() are not applicable"_s),
                    std::move(prepared.state));
            }
            auto request = buildRequest(config);
            const bool isMultipart
                = config.payloadKind == RequestBuilder::PayloadKind::MultipartFile
                  || config.payloadKind == RequestBuilder::PayloadKind::MultipartData;
            if (isMultipart && verb != "POST") {
                qCWarning(lcHttpClient).noquote()
                    << "multipart upload is always POST; verb" << verb << "ignored";
            }
            switch (config.payloadKind) {
            case RequestBuilder::PayloadKind::PutFile: {
                auto file = std::make_unique<QFile>(config.filePath);
                if (!file->open(QIODevice::ReadOnly)) {
                    return Prepared::fail(
                        fileError(config.url, config.filePath, file->errorString()),
                        std::move(prepared.state));
                }
                // 默认 PUT 走原生 put() 流式快路径；改写动词经自定义动词发送
                prepared.reply = verb == "PUT"
                                     ? q_ptr->put(request, file.get())
                                     : q_ptr->sendCustomRequest(request, verb, file.get());
                state->file = std::move(file);
                break;
            }
            case RequestBuilder::PayloadKind::PutData:
                prepared.reply = verb == "PUT"
                                     ? q_ptr->put(request, config.data)
                                     : q_ptr->sendCustomRequest(request, verb, config.data);
                break;
            case RequestBuilder::PayloadKind::MultipartFile: {
                auto file = std::make_unique<QFile>(config.filePath);
                if (!file->open(QIODevice::ReadOnly)) {
                    return Prepared::fail(
                        fileError(config.url, config.filePath, file->errorString()),
                        std::move(prepared.state));
                }
                const auto fileName = QFileInfo(config.filePath).fileName();
                prepared.reply = postMultipart(request, fileName, file.get(), {});
                state->file = std::move(file);
                break;
            }
            case RequestBuilder::PayloadKind::MultipartData:
                prepared.reply = postMultipart(request, config.fileName, nullptr, config.data);
                break;
            case RequestBuilder::PayloadKind::None:
                // 未配置载荷属 API 误用而非断言：debug/release 一致地返回 InvalidUse
                return Prepared::fail(invalidUse(config.url,
                                                 u"upload() requires a payload "
                                                 u"(putFile/putData/multipart*)"_s),
                                      std::move(prepared.state));
            }
            break;
        }
        }

        // 文件打开失败已在各分支提前返回，此处 reply 必非空
        connectSignals(config, prepared.reply, state.get());
        qCInfo(lcHttpClient).noquote()
            << state->method << redactedUrl(prepared.reply->url()) << "dispatched";
        return prepared;
    }

    static HttpResult buildResult(QNetworkReply *reply, const TaskState &state)
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

    HttpClient *q_ptr;
    // 生命周期契约：tasks 的键是非拥有的 reply 视图（reply 由 deleteLater 释放）。
    // 任何终止路径（onReplyFinished/onDownloadFinished/cancelInternal）必须先 erase
    // 再 reply->deleteLater()——reply 若在仍被键入时析构，地址一旦被新 reply 复用，
    // 陈旧条目会被误命中，TaskState 张冠李戴。cancelInternal 还须先 disconnect 再
    // abort（abort 会同步触发 finished，未断开将重入槽访问已移除的状态）
    std::unordered_map<QNetworkReply *, std::unique_ptr<TaskState>> tasks;

    // BasicCredentials 嵌套于此而非匿名 namespace：外部链接类的成员使用
    // 内部链接类型会触发 -Wsubobject-linkage（CI 有 -Werror）
    struct BasicCredentials
    {
        QString user;
        QString password;
    };

    QHash<QString, BasicCredentials> hostBasicAuth;
};

HttpClient::HttpClient(QObject *parent)
    : QNetworkAccessManager(parent), d_ptr(std::make_unique<HttpClientPrivate>(this))
{
    connect(this,
            &QNetworkAccessManager::authenticationRequired,
            this,
            &HttpClient::onAuthenticationRequired);
}

HttpClient::~HttpClient()
{
    // 析构时仍有未完成任务：onFinished 已不可能派发（静默丢弃），上层若依赖
    // 回调收尾会卡死。下载 .temp 保留到此：已 write 的字节随 QFile 析构 flush
    // 落盘（可信），仍留在 reply 读缓冲、尚未读出的字节随 reply 析构丢失。
    // 正常退出应在 delete 前显式 cancel()。
    if (d_ptr->tasks.empty()) {
        return;
    }
    qCWarning(lcHttpClient).noquote() << "HttpClient destroyed with" << qint64(d_ptr->tasks.size())
                                      << "in-flight request(s); callbacks dropped";
    for (const auto &[reply, state] : d_ptr->tasks) {
        const QByteArray dest = state->downloadPath.isEmpty()
                                    ? QByteArray{}
                                    : "-> "_ba + state->downloadPath.toUtf8();
        qCWarning(lcHttpClient).noquote()
            << "  dropped:" << state->method << redactedUrl(reply->url()) << dest;
    }
}

void HttpClient::resultReady(const HttpResult &) {}

// ── 请求工厂（无 body 版）──
RequestBuilder HttpClient::get(QUrl url)
{
    return RequestBuilder(
        this, RequestBuilder::Kind::Json, Method::Get, std::move(url), std::nullopt, {}, {});
}

RequestBuilder HttpClient::post(QUrl url)
{
    return RequestBuilder(
        this, RequestBuilder::Kind::Json, Method::Post, std::move(url), std::nullopt, {}, {});
}

RequestBuilder HttpClient::put(QUrl url)
{
    return RequestBuilder(
        this, RequestBuilder::Kind::Json, Method::Put, std::move(url), std::nullopt, {}, {});
}

RequestBuilder HttpClient::del(QUrl url)
{
    return RequestBuilder(
        this, RequestBuilder::Kind::Json, Method::Delete, std::move(url), std::nullopt, {}, {});
}

RequestBuilder HttpClient::request(Method method, QUrl url)
{
    return RequestBuilder(
        this, RequestBuilder::Kind::Json, method, std::move(url), std::nullopt, {}, {});
}

// ── 请求工厂（JSON body 便利版：序列化只发生在入口一次）──
RequestBuilder HttpClient::post(QUrl url, const QJsonObject &json)
{
    auto encoded = encodeJsonBody(json);
    return RequestBuilder(this,
                          RequestBuilder::Kind::Json,
                          Method::Post,
                          std::move(url),
                          std::move(encoded.data),
                          std::move(encoded.contentType),
                          {});
}

RequestBuilder HttpClient::put(QUrl url, const QJsonObject &json)
{
    auto encoded = encodeJsonBody(json);
    return RequestBuilder(this,
                          RequestBuilder::Kind::Json,
                          Method::Put,
                          std::move(url),
                          std::move(encoded.data),
                          std::move(encoded.contentType),
                          {});
}

RequestBuilder HttpClient::request(Method method, QUrl url, const QJsonObject &json)
{
    auto encoded = encodeJsonBody(json);
    return RequestBuilder(this,
                          RequestBuilder::Kind::Json,
                          method,
                          std::move(url),
                          std::move(encoded.data),
                          std::move(encoded.contentType),
                          {});
}

// ── download / upload ──
RequestBuilder HttpClient::download(QUrl url, QString filePath)
{
    return RequestBuilder(this,
                          RequestBuilder::Kind::Download,
                          Method::Get,
                          std::move(url),
                          std::nullopt,
                          {},
                          std::move(filePath));
}

RequestBuilder HttpClient::upload(QUrl url)
{
    return RequestBuilder(
        this, RequestBuilder::Kind::Upload, Method::Put, std::move(url), std::nullopt, {}, {});
}

HttpTask HttpClient::execute(RequestBuilder &builder)
{
    auto prepared = d_ptr->prepare(*builder.d_ptr);
    if (!prepared.ok()) {
        // 请求未发起（文件打开失败等）：与异步契约一致延迟到事件循环派发，
        // 避免回调在 send() 调用栈内同步执行；扩展点与回调同批派发
        auto callback = std::move(prepared.state->onFinished);
        auto error = prepared.error;
        error.method = prepared.state->method;
        QMetaObject::invokeMethod(
            this,
            [this, callback, error] {
                resultReady(error);
                callback(error);
            },
            Qt::QueuedConnection);
        return {};
    }
    d_ptr->tasks.emplace(prepared.reply, std::move(prepared.state));
    return HttpTask(this, prepared.reply);
}

HttpResult HttpClient::executeSync(RequestBuilder &builder)
{
    auto prepared = d_ptr->prepare(*builder.d_ptr);
    if (!prepared.ok()) {
        prepared.error.method = prepared.state->method;
        resultReady(prepared.error);
        return prepared.error;
    }

    HttpResult result;
    QEventLoop loop;
    // onFinished 改为内部接收器（结束事件循环）：用户 onFinished 无法交付——
    // 结果经返回值直接给调用方，回调会造成双重交付。
    // onProgress 保留：loop.exec() 阻塞期间 QNAM 信号正常派发，
    // 进度回调照常工作（同步下载大文件的进度上报依赖于此）
    prepared.state->onFinished = [&result, &loop](const HttpResult &r) {
        result = r;
        loop.quit();
    };
    // prepare 已发出请求，但信号派发要等事件循环启动，此时尚未 emplace 不会错过 finished
    d_ptr->tasks.emplace(prepared.reply, std::move(prepared.state));
    loop.exec();
    return result;
}

void HttpClient::cancelInternal(QNetworkReply *reply)
{
    if (reply == nullptr) {
        return;
    }
    const auto it = d_ptr->tasks.find(reply);
    if (it == d_ptr->tasks.end()) {
        return; // 已终止：幂等空操作
    }
    qCInfo(lcHttpClient).noquote()
        << it->second->method << redactedUrl(reply->url()) << "cancelled";
    // 顺序关键：先断开信号，再 abort（此时 TaskState 内文件仍有效），
    // 最后 erase 析构状态（停止定时器、关闭文件；下载保留 .temp 供续传）
    disconnect(reply, nullptr, this, nullptr);
    reply->abort();
    d_ptr->tasks.erase(it);
    reply->deleteLater();
}

void HttpClient::onSslErrors(const QList<QSslError> &errors)
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply == nullptr || errors.isEmpty()) {
        return;
    }
    const auto it = d_ptr->tasks.find(reply);
    if (it == d_ptr->tasks.end()) {
        return;
    }
    it->second->sslErrors.append(errors);
    if (it->second->ignoreSsl) {
        qCInfo(lcHttpClient) << "ignoring" << errors.size() << "SSL errors on"
                             << redactedUrl(reply->url());
        reply->ignoreSslErrors();
    } else {
        // 未要求忽略时不干预：QNAM 默认以 SslHandshakeFailedError 终止请求，
        // 错误详情已在上方收集，经 finished 时的 buildResult 带入 HttpResult
        qCWarning(lcHttpClient) << "SSL errors on" << redactedUrl(reply->url()) << errors;
    }
}

void HttpClient::onTimeout()
{
    auto *timer = qobject_cast<QTimer *>(sender());
    if (timer == nullptr) {
        return;
    }
    const auto it = std::ranges::find_if(d_ptr->tasks, [timer](const auto &pair) {
        return pair.second->timeoutTimer.get() == timer;
    });
    if (it == d_ptr->tasks.end()) {
        return;
    }
    qCWarning(lcHttpClient).noquote()
        << it->second->method << redactedUrl(it->first->url()) << "timed out after"
        << it->second->timeoutTimer->interval() << "ms";
    // 先置位再 abort：buildResult 据此把 OperationCanceledError 归因为超时，
    // 与 QNAM 内部取消（HTTP/2 RST_STREAM、代理取消等）区分
    it->second->timedOut = true;
    // abort 触发 finished，统一走结果装配路径，映射为 Timeout
    it->first->abort();
}

// 表值与 authenticator 现值相同则不回填：刚被拒的凭证原样重发只会重复
// 失败，交由 QNAM 以 AuthenticationRequiredError 终止
void HttpClient::onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    if (reply == nullptr || auth == nullptr) {
        return;
    }
    const auto it = d_ptr->hostBasicAuth.constFind(reply->url().host());
    if (it == d_ptr->hostBasicAuth.constEnd()) {
        return; // 该 host 无凭证：交由外部槽处理，或 QNAM 原生报错
    }
    if (auth->user() == it->user && auth->password() == it->password) {
        return;
    }
    auth->setUser(it->user);
    auth->setPassword(it->password);
    qCDebug(lcHttpClient).noquote()
        << "authentication challenge answered for" << redactedUrl(reply->url());
}

void HttpClient::onReplyFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply == nullptr) {
        return;
    }
    const auto it = d_ptr->tasks.find(reply);
    if (it == d_ptr->tasks.end()) {
        return;
    }
    auto state = std::move(it->second);
    d_ptr->tasks.erase(it);

    auto result = HttpClientPrivate::buildResult(reply, *state);
    // 超时 abort 后 device 已关闭，readAll 会告警；已收数据此时无意义
    if (reply->isOpen()) {
        result.body = reply->readAll();
        if (!result.body.isEmpty()) {
            result.json = QJsonDocument::fromJson(result.body);
        }
    }
    reply->deleteLater();
    qCInfo(lcHttpClient).noquote()
        << state->method << redactedUrl(result.url) << "finished"
        << "| status:" << result.status << "| code:" << qToUnderlying(result.code)
        << "| body:" << result.body.size() << "bytes";
    for (const auto &[name, value] : result.headers.toListOfPairs()) {
        qCDebug(lcHttpClient).noquote()
            << "response header:" << name << redactedHeader(name, value);
    }
    if (!result.body.isEmpty()) {
        // 完整输出：超长行/换行的清洗策略统一放在 messageHandler 层
        qCDebug(lcHttpClient).noquote() << "response body:" << result.body;
    }
    resultReady(result);
    state->onFinished(result);
}

void HttpClient::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply == nullptr) {
        return;
    }
    const auto it = d_ptr->tasks.find(reply);
    if (it == d_ptr->tasks.end()) {
        return;
    }
    it->second->onProgress(bytesSent, bytesTotal);
}

void HttpClient::onDownloadReadyRead()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply == nullptr) {
        return;
    }
    const auto it = d_ptr->tasks.find(reply);
    if (it == d_ptr->tasks.end() || it->second->file == nullptr) {
        return;
    }
    auto &state = it->second;
    QString writeError;
    if (!state->rangeChecked) {
        state->rangeChecked = true;
        if (state->downloadBase > 0) {
            const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (status == 200) {
                // 服务器忽略 Range 从头传输：截断 .temp 重写，避免拼接出损坏文件；
                // 截断失败（I/O 错误）断点不可信，与写入失败同路处理
                if (state->file->resize(0)) {
                    state->downloadBase = 0;
                } else {
                    writeError = state->file->errorString();
                    if (writeError.isEmpty()) {
                        writeError = u"resize failed"_s;
                    }
                }
            }
        }
    }
    if (writeError.isEmpty()) {
        writeError = HttpClientPrivate::drainToFile(reply, state->file.get(), state->ioChunk);
    }
    if (!writeError.isEmpty()) {
        // 截断/写入失败（磁盘满、I/O 错误）：停用流式写入，排队 abort
        // 交由 onDownloadFinished 派发 FileError
        state->fileError = writeError;
        disconnect(reply, &QNetworkReply::readyRead, this, nullptr);
        QMetaObject::invokeMethod(reply, [reply] { reply->abort(); }, Qt::QueuedConnection);
    }
}

void HttpClient::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply == nullptr) {
        return;
    }
    const auto it = d_ptr->tasks.find(reply);
    if (it == d_ptr->tasks.end()) {
        return;
    }
    const auto &state = it->second;
    const auto total = bytesTotal < 0 ? -1 : bytesTotal + state->downloadBase;
    state->onProgress(bytesReceived + state->downloadBase, total);
}

void HttpClient::onDownloadFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply == nullptr) {
        return;
    }
    const auto it = d_ptr->tasks.find(reply);
    if (it == d_ptr->tasks.end()) {
        return;
    }
    auto state = std::move(it->second);
    d_ptr->tasks.erase(it);

    auto result = HttpClientPrivate::buildResult(reply, *state);
    if (state->file) {
        // 仅 200/206 的体是文件数据；错误页/304/3xx 的体可能已随 readyRead
        // 写入 .temp，统一由下方分支清除
        const bool bodyIsFileData = result.status == 200 || result.status == 206;
        if (state->fileError.isEmpty() && result.success() && bodyIsFileData && reply->isOpen()) {
            // 空 200 不触发 readyRead，其中的截断逻辑落空：在此截断陈旧断点再
            // 排水（downloadBase 已归 0 则条件自灭），否则旧 .temp 会被原样
            // 交付；截断失败走 FileError 保留 .temp
            if (result.status == 200 && state->downloadBase > 0) {
                if (state->file->resize(0)) {
                    state->downloadBase = 0;
                } else {
                    state->fileError = state->file->errorString();
                    if (state->fileError.isEmpty()) {
                        state->fileError = u"resize failed"_s;
                    }
                }
            }
            if (state->fileError.isEmpty()) {
                // 末尾残余数据同样分块落盘（内存防护原因见 drainToFile）
                state->fileError
                    = HttpClientPrivate::drainToFile(reply, state->file.get(), state->ioChunk);
            }
        }
        const auto tempPath = state->file->fileName();
        // 末尾小块数据由 QFile 缓冲、close 时才真正落盘：flush 失败（磁盘满等）
        // 会把缺尾文件当成功交付，须在此捕获；非文件数据不查，反正会被丢弃
        if (state->fileError.isEmpty() && bodyIsFileData && !state->file->flush()) {
            state->fileError = state->file->errorString();
            if (state->fileError.isEmpty()) {
                state->fileError = u"flush failed"_s;
            }
        }
        state->file->close();
        if (!state->fileError.isEmpty()) {
            // 流式写入已失败（磁盘满等）：本地错误覆盖网络结果；保留 .temp 供续传
            result.code = HttpErrorCode::FileError;
            result.message = state->fileError;
            qCWarning(lcHttpClient).noquote() << "download file error:" << result.message;
        } else if (result.success() && result.status == 304) {
            // 条件命中：既有文件即有效内容，不覆盖；.temp 无本次数据，丢弃
            discardTempFile(tempPath, state->downloadBase);
            qCDebug(lcHttpClient).noquote()
                << "not modified; existing file kept, temp discarded:" << tempPath;
        } else if (result.success() && bodyIsFileData) {
            // 目标已存在：先移除再改名（QFile::rename 不覆盖既有文件）。
            // remove 失败时（Windows 上目标被占用、目标实为目录等）rename 必然
            // 随之失败——直接以 remove 失败报告 FileError，错误消息指向真实
            // 根因；完整数据保留在 .temp 可手工恢复
            if (QFile::exists(state->downloadPath)) {
                QFile target(state->downloadPath);
                if (!target.remove()) {
                    auto error = target.errorString();
                    if (error.isEmpty()) {
                        error = u"remove failed"_s;
                    }
                    result.code = HttpErrorCode::FileError;
                    result.message
                        = u"Cannot remove existing file %1: %2"_s.arg(state->downloadPath, error);
                    qCWarning(lcHttpClient).noquote() << result.message;
                }
            }
            if (result.success() && !state->file->rename(state->downloadPath)) {
                // 改名失败（目标被占用等）：完整数据保留在 .temp，报告 FileError 而非静默成功
                result.code = HttpErrorCode::FileError;
                result.message = u"Cannot save file %1: %2"_s.arg(state->downloadPath,
                                                                  state->file->errorString());
                qCWarning(lcHttpClient).noquote() << result.message;
            }
        } else if (result.status >= 400) {
            // HTTP 错误状态（4xx/5xx，含 416）：错误页可能已随 readyRead 写入
            // .temp，断点不可信，整个丢弃（见 discardTempFile），重试从头下载
            discardTempFile(tempPath, state->downloadBase);
            qCDebug(lcHttpClient).noquote() << "HTTP error status; temp discarded:" << tempPath;
        } else if (result.success()) {
            // 未跟随的 3xx/204 等：响应体不是文件数据，.temp 同样丢弃
            discardTempFile(tempPath, state->downloadBase);
            qCDebug(lcHttpClient).noquote() << "non-file response; temp discarded:" << tempPath;
        } else {
            // 传输中途失败（头已到达、error 非 NoError，status 为 200/206）或
            // 头未到达（status==0）：.temp 内是真实数据，保留供断点续传
            qCDebug(lcHttpClient).noquote() << "temp kept:" << tempPath;
        }
    }
    reply->deleteLater();
    qCInfo(lcHttpClient).noquote()
        << state->method << redactedUrl(result.url) << "finished"
        << "| status:" << result.status << "| code:" << qToUnderlying(result.code)
        << "| file:" << state->downloadPath;
    resultReady(result);
    state->onFinished(result);
}

HttpTask::HttpTask(HttpClient *client, QNetworkReply *reply)
    : d_ptr(std::make_shared<HttpTaskPrivate>())
{
    d_ptr->client = client;
    d_ptr->reply = reply;
}

void HttpTask::cancel() const
{
    if (d_ptr && d_ptr->client && d_ptr->reply) {
        d_ptr->client->cancelInternal(d_ptr->reply);
    }
}

RequestBuilder::RequestBuilder(HttpClient *client,
                               Kind kind,
                               HttpClient::Method method,
                               QUrl url,
                               std::optional<QByteArray> body,
                               QByteArray contentType,
                               QString downloadPath)
    : d_ptr(std::make_unique<RequestBuilderPrivate>())
{
    d_ptr->client = client;
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
    d_ptr->client->d_ptr->hostBasicAuth[d_ptr->url.host()] = {user, password};
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
    if (std::exchange(d_ptr->consumed, true)) {
        qCWarning(lcHttpClient)
            << u"RequestBuilder::send(): builder already used; request not sent"_s;
        return {};
    }
    return d_ptr->client->execute(*this);
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
    return d_ptr->client->executeSync(*this);
}
