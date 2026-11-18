#include "httpclient_p.hpp"

#include <QAuthenticator>
#include <QEventLoop>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QLoggingCategory>
#include <QMetaEnum>
#include <QPointer>
#include <QStringView>
#include <QTimer>
#include <QUrlQuery>

#include <algorithm>
#include <array>
#include <ranges>
#include <utility>

using namespace Qt::StringLiterals;
using Utils::encodeJsonBody;
using Utils::Prepared;
using Utils::redactedHeader;
using Utils::TaskState;

// 分类日志：默认只输出里程碑（info）与错误（warning）；
// 设 QT_LOGGING_RULES="httpclient.debug=true" 可开启请求/响应明细，便于调试
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

qint64 effectiveReadBufferLimit(qint64 limit)
{ return limit > 0 ? limit : kDownloadReadBufferLimit; }

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

// 常规流水（dispatched/cancelled/成功结束）的 URL 通道：userinfo 密码与 query
// 凭证值替换为占位符（与敏感头的 *** 约定一致），用户名与非敏感参数保留。
// 这些行默认输出、不受 debug 开关控制，故在输出前脱敏。
// 失败日志刻意不走此处（见 logFinished）：排障需要完整现场，不要"统一"
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

// 本地扩展码由 switch 全覆盖（新增枚举会 -Wswitch 报错）；其余复用 QNetworkReply 的 Q_ENUM
QByteArray errorCodeName(HttpErrorCode code)
{
    switch (code) {
    case HttpErrorCode::NoError: return "NoError"_ba;
    case HttpErrorCode::Timeout: return "Timeout"_ba;
    case HttpErrorCode::FileError: return "FileError"_ba;
    case HttpErrorCode::InvalidUse: return "InvalidUse"_ba;
    case HttpErrorCode::Canceled: return "Canceled"_ba;
    }
    const int value = qToUnderlying(code);
    const char *name = QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(value);
    return name != nullptr ? QByteArray(name) : QByteArray::number(value);
}

// 重定向时给出"发起地址 -> 最终地址"；redact 走常规流水的脱敏通道，否则原文
QString urlForLog(const HttpResult &result, bool redact)
{
    if (result.request.url == result.url) {
        return redact ? redactedUrl(result.url) : result.url.toString();
    }
    const QString from = redact ? redactedUrl(result.request.url) : result.request.url.toString();
    return from + u" -> "_s + (redact ? redactedUrl(result.url) : result.url.toString());
}

// 终止日志唯一出口（两个终止槽共用）：成功 info + debug 明细，失败 warning 全上下文。
// 正文整段输出，行长度归 sink；失败行不脱敏（排障要现场），常规流水仍走 redactedUrl
void logFinished(const HttpResult &result)
{
    const QByteArray codeName = errorCodeName(result.code);

    if (result.success()) {
        qCInfo(lcHttpClient).noquote()
            << result.request.method << urlForLog(result, true) << "finished"
            << "| status:" << result.status << "| code:" << codeName
            << "| body:" << result.body.size() << "bytes";
        for (const auto &[name, value] : result.headers.toListOfPairs()) {
            qCDebug(lcHttpClient).noquote()
                << "response header:" << name << redactedHeader(name, value);
        }
        if (!result.body.isEmpty()) {
            // 完整输出：超长行/换行的清洗策略统一放在 messageHandler/sink 层
            qCDebug(lcHttpClient).noquote() << "response body:" << result.body;
        }
        return;
    }

    qCWarning(lcHttpClient).noquote()
        << result.request.method << urlForLog(result, false) << "failed"
        << "| status:" << result.status << "| code:" << codeName
        << "(" + QByteArray::number(qToUnderlying(result.code)) + ")";
    // FileError 的消息已在产生它的收尾分支打过，这里不再重复一遍
    if (result.code != HttpErrorCode::FileError) {
        qCWarning(lcHttpClient).noquote() << "  reason:" << result.message;
    }
    if (!result.sslErrors.isEmpty()) {
        qCWarning(lcHttpClient) << "  ssl errors:" << result.sslErrors;
    }
    for (const auto &[name, value] : result.request.headers.toListOfPairs()) {
        qCWarning(lcHttpClient).noquote() << "  > header:" << name << value;
    }
    if (!result.request.body.isEmpty()) {
        qCWarning(lcHttpClient).noquote() << "  > body:" << result.request.body;
    }
    for (const auto &[name, value] : result.headers.toListOfPairs()) {
        qCWarning(lcHttpClient).noquote() << "  < header:" << name << value;
    }
    if (!result.body.isEmpty()) {
        qCWarning(lcHttpClient).noquote() << "  < body:" << result.body;
    }
}

} // namespace

QList<QByteArray> HttpResult::header(QByteArrayView name) const
{ return headers.values(name); }

// 取消句柄的内部状态：QPointer 弱引用，不延长任何生命周期
class HttpTask::HttpTaskPrivate
{
public:
    HttpTaskPrivate() = default;

    QPointer<HttpClient> client;
    QPointer<QNetworkReply> reply;
};

HttpClient::HttpClient(QObject *parent)
    : QNetworkAccessManager(parent), d_ptr(std::make_unique<HttpClientPrivate>())
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
            << "  dropped:" << state->request.method << redactedUrl(reply->url()) << dest;
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
    auto prepared = prepare(builder);
    if (!prepared.ok()) {
        // 请求未发起（文件打开失败等）：与异步契约一致延迟到事件循环派发，
        // 避免回调在 send() 调用栈内同步执行；扩展点与回调同批派发
        auto callback = std::move(prepared.state->onFinished);
        auto error = prepared.error;
        error.request.method = prepared.state->request.method;
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
    auto prepared = prepare(builder);
    if (!prepared.ok()) {
        prepared.error.request.method = prepared.state->request.method;
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
        << it->second->request.method << redactedUrl(reply->url()) << "cancelled";
    // 顺序关键：先断开信号，再 abort（此时 TaskState 内文件仍有效），
    // 最后 erase 析构状态（停止定时器、关闭文件；下载保留 .temp 供续传）
    disconnect(reply, nullptr, this, nullptr);
    reply->abort();
    d_ptr->tasks.erase(it);
    reply->deleteLater();
}

void HttpClient::registerBasicAuth(const QString &host,
                                   const QString &user,
                                   const QString &password)
{ d_ptr->hostBasicAuth[host] = {user, password}; }

std::unique_ptr<TaskState> HttpClient::buildTaskState(RequestBuilder &builder)
{
    const auto &config = *builder.d_ptr;
    auto state = std::make_unique<TaskState>();
    state->request.method = HttpClientPrivate::effectiveVerb(config);
    state->onFinished = config.onFinished;
    state->onProgress = config.onProgress;
    state->ignoreSsl = config.ignoreSslErrors;
    if (config.timeout > std::chrono::milliseconds{0}) {
        state->timeoutTimer = std::make_unique<QTimer>();
        state->timeoutTimer->setSingleShot(true);
        connect(state->timeoutTimer.get(), &QTimer::timeout, this, &HttpClient::onTimeout);
        state->timeoutTimer->start(config.timeout);
    }
    return state;
}

Prepared HttpClient::prepare(RequestBuilder &builder)
{
    const auto &config = *builder.d_ptr;
    Prepared prepared;
    prepared.state = buildTaskState(builder);
    auto &state = prepared.state;

    const auto verb = HttpClientPrivate::effectiveVerb(config);
    // body()/json() 的载荷；Upload 的载荷由 putFile/multipart* 独占（此处有值即误用）
    const QByteArray body = config.bodyData.value_or(QByteArray{});
    if (!body.isEmpty()) {
        qCDebug(lcHttpClient).noquote() << "request body:" << body;
    }

    QNetworkRequest request;
    switch (config.kind) {
    case RequestBuilder::Kind::Json:
        request = d_ptr->buildRequest(config);
        prepared.reply = sendCustomRequest(request, verb, body);
        break;
    case RequestBuilder::Kind::Download: {
        const auto tempPath = config.downloadPath + u".temp"_s;
        auto file = std::make_unique<QFile>(tempPath);
        if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
            return Prepared::fail(
                HttpClientPrivate::fileError(config.url, tempPath, file->errorString()),
                std::move(prepared.state));
        }
        request = d_ptr->buildRequest(config, "Range");
        state->downloadBase = file->size();
        if (state->downloadBase > 0) {
            request.setRawHeader("Range",
                                 "bytes="_ba + QByteArray::number(state->downloadBase) + "-"_ba);
        }
        // 纯 GET 走原生 get() 快路径；改动词或带动作体经自定义动词发送
        const bool needCustomVerb = config.bodyData.has_value() || !config.verbOverride.isEmpty()
                                    || config.method != HttpClient::Method::Get;
        if (needCustomVerb) {
            prepared.reply = sendCustomRequest(request, verb, body);
        } else {
            prepared.reply = get(request);
        }
        // 背压窗口需排队设置：setReadBufferSize() 经信号通知 HTTP 线程 delegate，
        // 而信号连接在请求启动（_q_startOperation，get() 后的排队事件）才建立，
        // 同步调用会因信号无接收者而失效
        const auto readBufferLimit = effectiveReadBufferLimit(config.readBufferLimit);
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
                HttpClientPrivate::invalidUse(
                    config.url,
                    u"upload() payload is provided by putFile/putData/multipart*; "
                    u"body()/json() are not applicable"_s),
                std::move(prepared.state));
        }
        request = d_ptr->buildRequest(config);
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
                    HttpClientPrivate::fileError(config.url, config.filePath, file->errorString()),
                    std::move(prepared.state));
            }
            // 默认 PUT 走原生 put() 流式快路径；改写动词经自定义动词发送
            prepared.reply = verb == "PUT" ? put(request, file.get())
                                           : sendCustomRequest(request, verb, file.get());
            state->file = std::move(file);
            break;
        }
        case RequestBuilder::PayloadKind::PutData:
            prepared.reply = verb == "PUT" ? put(request, config.data)
                                           : sendCustomRequest(request, verb, config.data);
            break;
        case RequestBuilder::PayloadKind::MultipartFile: {
            auto file = std::make_unique<QFile>(config.filePath);
            if (!file->open(QIODevice::ReadOnly)) {
                return Prepared::fail(
                    HttpClientPrivate::fileError(config.url, config.filePath, file->errorString()),
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
            return Prepared::fail(HttpClientPrivate::invalidUse(config.url,
                                                                u"upload() requires a payload "
                                                                u"(putFile/putData/multipart*)"_s),
                                  std::move(prepared.state));
        }
        break;
    }
    }

    // 请求到此才是最终形态（含下载续传的 Range）：补齐快照，body 必须复制
    state->request.url = request.url();
    state->request.headers = request.headers();
    state->request.body = body;
    // 文件打开失败已在各分支提前返回，此处 reply 必非空
    connectSignals(builder, prepared.reply, state.get());
    qCInfo(lcHttpClient).noquote()
        << state->request.method << redactedUrl(prepared.reply->url()) << "dispatched";
    return prepared;
}

void HttpClient::connectSignals(RequestBuilder &builder, QNetworkReply *reply, TaskState *state)
{
    const auto &config = *builder.d_ptr;
    connect(reply, &QNetworkReply::sslErrors, this, &HttpClient::onSslErrors);
    switch (config.kind) {
    case RequestBuilder::Kind::Json:
    case RequestBuilder::Kind::Upload:
        connect(reply, &QNetworkReply::finished, this, &HttpClient::onReplyFinished);
        if (state->onProgress) {
            connect(reply, &QNetworkReply::uploadProgress, this, &HttpClient::onUploadProgress);
        }
        break;
    case RequestBuilder::Kind::Download:
        connect(reply, &QNetworkReply::finished, this, &HttpClient::onDownloadFinished);
        connect(reply, &QNetworkReply::readyRead, this, &HttpClient::onDownloadReadyRead);
        // 重定向会重建 HTTP 线程 delegate（窗口归零），每个响应到达时重设背压
        const auto readBufferLimit = effectiveReadBufferLimit(config.readBufferLimit);
        connect(reply, &QNetworkReply::metaDataChanged, reply, [reply, readBufferLimit] {
            reply->setReadBufferSize(readBufferLimit);
        });
        if (state->onProgress) {
            connect(reply, &QNetworkReply::downloadProgress, this, &HttpClient::onDownloadProgress);
        }
        break;
    }
}

QNetworkReply *HttpClient::postMultipart(const QNetworkRequest &request,
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
    auto *reply = post(request, multiPart);
    multiPart->setParent(reply);
    return reply;
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
        << it->second->request.method << redactedUrl(it->first->url()) << "timed out after"
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
    logFinished(result);
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
            // HTTP 错误状态（4xx/5xx，含 416）：错误页可能已随 readyRead 写入 .temp，
            // 断点不可信，整个丢弃（见 discardTempFile）。错误页内容不回读：响应头里
            // 有 Content-Type/Content-Length，需要正文时用同一请求复现即可
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
    logFinished(result);
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
