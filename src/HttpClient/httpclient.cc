#include "httpclient.hpp"

#include <QEventLoop>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QPointer>
#include <QTimer>
#include <QUrlQuery>

#include <algorithm>
#include <ranges>
#include <unordered_map>
#include <utility>

using namespace Qt::StringLiterals;

// 本地错误码的可读消息；网络错误不经过此函数，消息直接取 QNetworkReply::errorString()
static QString httpErrorMessage(HttpErrorCode code)
{
    switch (code) {
    case HttpErrorCode::Timeout: return u"Request timed out"_s;
    case HttpErrorCode::FileError: return u"File operation failed"_s;
    default: return {};
    }
}

QByteArray HttpResult::header(QByteArrayView name) const
{
    const auto wanted = QString::fromLatin1(name);
    for (const auto &[headerName, value] : std::as_const(headers)) {
        if (QString::fromLatin1(headerName).compare(wanted, Qt::CaseInsensitive) == 0) {
            return value;
        }
    }
    return {};
}

class RequestBuilder::Private
{
public:
    HttpClient *client = nullptr;
    RequestBuilder::Kind kind = RequestBuilder::Kind::Json;
    HttpClient::Method method = HttpClient::Method::Get;
    QUrl url;
    QJsonObject json;
    QString downloadPath;

    RequestBuilder::PayloadKind payloadKind = RequestBuilder::PayloadKind::None;
    QString filePath;
    QByteArray data;
    QString fileName;

    QList<QPair<QByteArray, QByteArray>> headers;
    QList<QPair<QString, QString>> queries;
    std::chrono::milliseconds timeout{0};
    bool ignoreSslErrors = false;
    bool consumed = false; // send()/sync() 一次性守卫
    HttpFinishedCallback onFinished;
    HttpProgressCallback onProgress;
};

// 取消句柄的内部状态：QPointer 弱引用，不延长任何生命周期
class HttpTask::Private
{
public:
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
        std::unique_ptr<QTimer> timeoutTimer; // 总时长超时，随析构自动停止
        QString downloadPath;                 // 仅下载：最终路径
        qint64 downloadBase = 0;              // 仅下载：断点续传偏移
        bool ignoreSsl = false;
        bool rangeChecked = false;            // 仅下载：首个 readyRead 已处理 200/206 分支
        QString fileError;                    // 仅下载：写入/保存失败原因；非空即失败
        // QNetworkReply 未提供 sslErrors() getter（至少 Qt 6.11 确认无），
        // sslErrors 信号是唯一来源：槽中收集，buildResult 时带入结果
        QList<QSslError> sslErrors;
    };

    // prepare 的产出：reply 为空表示请求未发起（文件打开失败等），error 携带原因
    struct Prepared
    {
        QNetworkReply *reply = nullptr;
        std::unique_ptr<TaskState> state;
        HttpResult error;

        [[nodiscard]] bool ok() const { return reply != nullptr; }
    };

    static QByteArray methodBytes(HttpClient::Method method)
    {
        // Q_ENUM 反射取得枚举名（"Get"），统一大写化为 HTTP 动词（"GET"）
        return QByteArray(
                   QMetaEnum::fromType<HttpClient::Method>().valueToKey(static_cast<int>(method)))
            .toUpper();
    }

    static HttpResult fileError(const QUrl &url, const QString &path, const QString &reason)
    {
        HttpResult result;
        result.code = HttpErrorCode::FileError;
        result.url = url;
        result.message = u"Cannot open file %1: %2"_s.arg(path, reason);
        return result;
    }

    QNetworkRequest buildRequest(const RequestBuilder::Private &config)
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
        for (const auto &[name, value] : std::as_const(config.headers)) {
            request.setRawHeader(name, value);
        }
        return request;
    }

    // 构建 TaskState：回调与超时定时器统一在此装配，各请求路径共享
    std::unique_ptr<TaskState> buildTaskState(const RequestBuilder::Private &config)
    {
        auto state = std::make_unique<TaskState>();
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

    QNetworkReply *postMultipart(const QNetworkRequest &request,
                                 const QString &fileName,
                                 QIODevice *device,
                                 const QByteArray &data)
    {
        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           u"form-data; name=\"file\"; filename=\"%1\""_s.arg(fileName));
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

    void
    connectSignals(const RequestBuilder::Private &config, QNetworkReply *reply, TaskState *state)
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
            if (state->onProgress) {
                q_ptr->connect(reply,
                               &QNetworkReply::downloadProgress,
                               q_ptr,
                               &HttpClient::onDownloadProgress);
            }
            break;
        }
    }

    Prepared prepare(const RequestBuilder::Private &config)
    {
        Prepared prepared;
        prepared.state = buildTaskState(config);
        auto &state = prepared.state;

        switch (config.kind) {
        case RequestBuilder::Kind::Json: {
            auto request = buildRequest(config);
            QByteArray body;
            if (!config.json.isEmpty()) {
                request.setHeader(QNetworkRequest::ContentTypeHeader,
                                  u"application/json; charset=utf-8"_s);
                body = QJsonDocument(config.json).toJson(QJsonDocument::Compact);
            }
            prepared.reply = q_ptr->sendCustomRequest(request, methodBytes(config.method), body);
            break;
        }
        case RequestBuilder::Kind::Download: {
            const auto tempPath = config.downloadPath + u".temp"_s;
            auto file = std::make_unique<QFile>(tempPath);
            if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
                prepared.error = fileError(config.url, tempPath, file->errorString());
                return prepared;
            }
            auto request = buildRequest(config);
            state->downloadBase = file->size();
            if (state->downloadBase > 0) {
                request.setRawHeader(
                    "Range", "bytes="_ba + QByteArray::number(state->downloadBase) + "-"_ba);
            }
            prepared.reply = q_ptr->get(request);
            state->file = std::move(file);
            state->downloadPath = config.downloadPath;
            break;
        }
        case RequestBuilder::Kind::Upload: {
            auto request = buildRequest(config);
            switch (config.payloadKind) {
            case RequestBuilder::PayloadKind::PutFile: {
                auto file = std::make_unique<QFile>(config.filePath);
                if (!file->open(QIODevice::ReadOnly)) {
                    prepared.error = fileError(config.url, config.filePath, file->errorString());
                    return prepared;
                }
                prepared.reply = q_ptr->put(request, file.get());
                state->file = std::move(file);
                break;
            }
            case RequestBuilder::PayloadKind::PutData:
                prepared.reply = q_ptr->put(request, config.data);
                break;
            case RequestBuilder::PayloadKind::MultipartFile: {
                auto file = std::make_unique<QFile>(config.filePath);
                if (!file->open(QIODevice::ReadOnly)) {
                    prepared.error = fileError(config.url, config.filePath, file->errorString());
                    return prepared;
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
                // 未配置载荷属运行期错误而非断言：debug/release 一致地返回 FileError
                prepared.error.code = HttpErrorCode::FileError;
                prepared.error.url = config.url;
                prepared.error.message = u"upload() requires a payload"_s;
                return prepared;
            }
            break;
        }
        }

        // 文件打开失败已在各分支提前返回，此处 reply 必非空
        connectSignals(config, prepared.reply, state.get());
        return prepared;
    }

    static HttpResult buildResult(QNetworkReply *reply, const TaskState &state)
    {
        HttpResult result;
        result.url = reply->url();
        result.status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result.headers = reply->rawHeaderPairs();
        result.sslErrors = state.sslErrors;
        const auto error = reply->error();
        if (error == QNetworkReply::NoError) {
            result.code = HttpErrorCode::NoError;
        } else if (error == QNetworkReply::OperationCanceledError) {
            // 取消路径会先 disconnect 再 abort，不会进入槽；
            // 未被取消的 OperationCanceledError 只能来自超时定时器的 abort
            result.code = HttpErrorCode::Timeout;
            result.message = httpErrorMessage(HttpErrorCode::Timeout);
        } else {
            // 网络层失败与服务器错误状态（QNAM 映射码），消息直接用 errorString()
            result.code = static_cast<HttpErrorCode>(error);
            result.message = reply->errorString();
        }
        return result;
    }

    HttpClient *q_ptr;
    std::unordered_map<QNetworkReply *, std::unique_ptr<TaskState>> tasks;
};

HttpClient::HttpClient(QObject *parent)
    : QNetworkAccessManager(parent), d_ptr(std::make_unique<HttpClientPrivate>(this))
{}

HttpClient::~HttpClient() = default;

RequestBuilder HttpClient::get(QUrl url)
{ return RequestBuilder(this, RequestBuilder::Kind::Json, Method::Get, std::move(url), {}, {}); }

RequestBuilder HttpClient::post(QUrl url, const QJsonObject &json)
{ return RequestBuilder(this, RequestBuilder::Kind::Json, Method::Post, std::move(url), json, {}); }

RequestBuilder HttpClient::put(QUrl url, const QJsonObject &json)
{ return RequestBuilder(this, RequestBuilder::Kind::Json, Method::Put, std::move(url), json, {}); }

RequestBuilder HttpClient::del(QUrl url)
{ return RequestBuilder(this, RequestBuilder::Kind::Json, Method::Delete, std::move(url), {}, {}); }

RequestBuilder HttpClient::request(Method method, QUrl url, const QJsonObject &json)
{ return RequestBuilder(this, RequestBuilder::Kind::Json, method, std::move(url), json, {}); }

RequestBuilder HttpClient::download(QUrl url, QString filePath)
{
    return RequestBuilder(
        this, RequestBuilder::Kind::Download, Method::Get, std::move(url), {}, std::move(filePath));
}

RequestBuilder HttpClient::upload(QUrl url)
{ return RequestBuilder(this, RequestBuilder::Kind::Upload, Method::Put, std::move(url), {}, {}); }

HttpTask HttpClient::execute(RequestBuilder &builder)
{
    auto prepared = d_ptr->prepare(*builder.d_ptr);
    if (!prepared.ok()) {
        // 请求未发起（文件打开失败等）：与异步契约一致延迟到事件循环派发，
        // 避免回调在 send() 调用栈内同步执行
        auto callback = std::move(prepared.state->onFinished);
        const auto error = prepared.error;
        QMetaObject::invokeMethod(
            this, [callback, error] { callback(error); }, Qt::QueuedConnection);
        return {};
    }
    d_ptr->tasks.emplace(prepared.reply, std::move(prepared.state));
    return HttpTask(this, prepared.reply);
}

HttpResult HttpClient::executeSync(RequestBuilder &builder)
{
    auto prepared = d_ptr->prepare(*builder.d_ptr);
    if (!prepared.ok()) {
        return prepared.error;
    }

    HttpResult result;
    QEventLoop loop;
    // sync 忽略用户回调：onFinished 改为内部接收器，onProgress 置空
    prepared.state->onProgress = {};
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
        reply->ignoreSslErrors();
    }
    // 未要求忽略时不干预：QNAM 默认以 SslHandshakeFailedError 终止请求，
    // 错误详情已在上方收集，经 finished 时的 buildResult 带入 HttpResult
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
    // abort 触发 finished，统一走结果装配路径，映射为 Timeout
    it->first->abort();
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
    if (!state->rangeChecked) {
        state->rangeChecked = true;
        if (state->downloadBase > 0) {
            const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (status == 200) {
                // 服务器忽略 Range 从头传输：截断 .temp 重写，避免拼接出损坏文件
                state->file->resize(0);
                state->downloadBase = 0;
            }
        }
    }
    const auto data = reply->readAll();
    if (state->file->write(data) != data.size()) {
        // 磁盘满等写入失败：停用流式写入，排队 abort 交由 onDownloadFinished 派发 FileError
        state->fileError = state->file->errorString();
        if (state->fileError.isEmpty()) {
            state->fileError = u"write failed"_s;
        }
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
        if (state->fileError.isEmpty() && result.success() && reply->isOpen()) {
            const auto data = reply->readAll();
            if (state->file->write(data) != data.size()) {
                state->fileError = state->file->errorString();
                if (state->fileError.isEmpty()) {
                    state->fileError = u"write failed"_s;
                }
            }
        }
        const auto tempPath = state->file->fileName();
        state->file->close();
        if (!state->fileError.isEmpty()) {
            // 流式写入已失败（磁盘满等）：本地错误覆盖网络结果；保留 .temp 供续传
            result.code = HttpErrorCode::FileError;
            result.message = state->fileError;
        } else if (result.success()) {
            if (QFile::exists(state->downloadPath) && !QFile::remove(state->downloadPath)) {
                qWarning() << u"Failed to remove existing file:"_s << state->downloadPath;
            }
            if (!state->file->rename(state->downloadPath)) {
                // 改名失败（目标被占用等）：完整数据保留在 .temp，报告 FileError 而非静默成功
                result.code = HttpErrorCode::FileError;
                result.message = u"Cannot save file %1: %2"_s.arg(state->downloadPath,
                                                                  state->file->errorString());
            }
        } else if (result.status == 416) {
            // Range 不可满足：.temp 或已完整或与服务器内容不一致，保留待用户处置
        } else if (result.status > 0) {
            // HTTP 错误响应（4xx/5xx）：readyRead 流式写入的是错误页而非文件数据，
            // .temp 已被污染必须丢弃；网络层失败（status==0）才保留供断点续传
            QFile::remove(tempPath);
        }
    }
    reply->deleteLater();
    state->onFinished(result);
}

HttpTask::HttpTask(HttpClient *client, QNetworkReply *reply) : d_ptr(std::make_shared<Private>())
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
                               QJsonObject json,
                               QString downloadPath)
    : d_ptr(std::make_unique<Private>())
{
    d_ptr->client = client;
    d_ptr->kind = kind;
    d_ptr->method = method;
    d_ptr->url = std::move(url);
    d_ptr->json = std::move(json);
    d_ptr->downloadPath = std::move(downloadPath);
}

RequestBuilder::RequestBuilder(RequestBuilder &&) noexcept = default;
RequestBuilder &RequestBuilder::operator=(RequestBuilder &&) noexcept = default;
RequestBuilder::~RequestBuilder() = default;

RequestBuilder &RequestBuilder::header(QByteArray name, QByteArray value)
{
    d_ptr->headers.append({std::move(name), std::move(value)});
    return *this;
}

RequestBuilder &RequestBuilder::bearer(QByteArrayView token)
{ return header("Authorization", "Bearer " + token.toByteArray()); }

RequestBuilder &RequestBuilder::basicAuth(const QString &user, const QString &password)
{
    const auto credentials = u"%1:%2"_s.arg(user, password).toUtf8().toBase64();
    return header("Authorization", "Basic " + credentials);
}

RequestBuilder &RequestBuilder::query(const QString &key, const QString &value)
{
    d_ptr->queries.append({key, value});
    return *this;
}

RequestBuilder &RequestBuilder::timeout(std::chrono::milliseconds timeout)
{
    d_ptr->timeout = timeout;
    return *this;
}

RequestBuilder &RequestBuilder::ignoreSslErrors()
{
    d_ptr->ignoreSslErrors = true;
    return *this;
}

RequestBuilder &RequestBuilder::onFinished(HttpFinishedCallback callback)
{
    d_ptr->onFinished = std::move(callback);
    return *this;
}

RequestBuilder &RequestBuilder::onProgress(HttpProgressCallback callback)
{
    d_ptr->onProgress = std::move(callback);
    return *this;
}

RequestBuilder &RequestBuilder::putFile(QString filePath)
{
    d_ptr->payloadKind = PayloadKind::PutFile;
    d_ptr->filePath = std::move(filePath);
    return *this;
}

RequestBuilder &RequestBuilder::putData(QByteArray data)
{
    d_ptr->payloadKind = PayloadKind::PutData;
    d_ptr->data = std::move(data);
    return *this;
}

RequestBuilder &RequestBuilder::multipartFile(QString filePath)
{
    d_ptr->payloadKind = PayloadKind::MultipartFile;
    d_ptr->filePath = std::move(filePath);
    return *this;
}

RequestBuilder &RequestBuilder::multipartData(QString fileName, QByteArray data)
{
    d_ptr->payloadKind = PayloadKind::MultipartData;
    d_ptr->fileName = std::move(fileName);
    d_ptr->data = std::move(data);
    return *this;
}

HttpTask RequestBuilder::send()
{
    if (std::exchange(d_ptr->consumed, true)) {
        // 二次终止属 API 误用：告警并忽略，不发起第二个请求
        qWarning() << u"RequestBuilder::send(): builder already used; request not sent"_s;
        return {};
    }
    return d_ptr->client->execute(*this);
}

HttpResult RequestBuilder::sync()
{
    if (std::exchange(d_ptr->consumed, true)) {
        HttpResult result;
        result.code = HttpErrorCode::InvalidUse;
        result.message = u"Builder already used"_s;
        return result;
    }
    return d_ptr->client->executeSync(*this);
}
