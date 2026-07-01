#include "httpclient.hpp"

#include <QEventLoop>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QTimer>

#include <algorithm>
#include <ranges>
#include <unordered_map>

using namespace Qt::StringLiterals;

class HttpClient::HttpClientPrivate
{
public:
    explicit HttpClientPrivate(HttpClient *q) : q_ptr(q)
    { sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone); }

    QNetworkRequest networkRequest(bool verifyCertificate)
    {
        QNetworkRequest request;
        if (!verifyCertificate) {
            request.setSslConfiguration(sslConfiguration);
        }
        return request;
    }

    static QJsonObject jsonFromBytes(const QByteArray &bytes)
    {
        QJsonParseError jsonParseError;
        auto jsonDocument = QJsonDocument::fromJson(bytes, &jsonParseError);
        if (QJsonParseError::NoError != jsonParseError.error) {
            qWarning() << jsonParseError.errorString() << u"\nOffset: "_s << jsonParseError.offset
                       << bytes;
            return {};
        }
        return jsonDocument.object();
    }

    // 利用 Q_ENUM 反射：Method 的字符串名直接由元对象系统提供，无需手写 switch
    static QByteArray methodToByteArray(Method method)
    { return QMetaEnum::fromType<Method>().valueToKey(static_cast<int>(method)); }

    struct TaskContext
    {
        HttpRequestOptions::JsonCallback callback;
        HttpRequestOptions::ProgressCallback progressCallback;
        HttpRequestOptions::ResultHook resultHook;
        std::unique_ptr<QFile> file;          // 上传：输入文件；下载：输出文件
        std::unique_ptr<QTimer> timeoutTimer; // 超时定时器，随 TaskContext 析构自动停止并销毁
        QString downloadPath;                 // 仅下载非空（finish 时 rename 目标）
        qint64 fileBaseSize = 0;              // 仅下载（断点续传偏移）

        // 下载完成：写剩余数据、关闭文件、重命名临时文件到最终路径
        void finalizeDownload(QNetworkReply *reply)
        {
            if (!file)
                return;
            file->write(reply->readAll());
            file->close(); // close 内部会 flush，无需手动调用
            if (QFile::exists(downloadPath)) {
                if (!QFile::remove(downloadPath)) {
                    qWarning() << u"Failed to remove existing file:"_s << downloadPath;
                }
            }
            if (!file->rename(downloadPath)) {
                qWarning() << u"Failed to rename temp file to:"_s << downloadPath;
            }
        }
    };

    // 统一构建 TaskContext：填充回调与超时定时器，避免在调用方重复样板代码
    std::unique_ptr<TaskContext> buildTaskContext(const HttpRequestOptions &options)
    {
        auto ctx = std::make_unique<TaskContext>();
        ctx->callback = options.callback;
        ctx->progressCallback = options.progressCallback;
        ctx->resultHook = options.resultHook;
        if (options.timeout > 0) {
            ctx->timeoutTimer = std::make_unique<QTimer>();
            ctx->timeoutTimer->setSingleShot(true);
            q_ptr->connect(
                ctx->timeoutTimer.get(), &QTimer::timeout, q_ptr, &HttpClient::onNetworkTimeout);
            ctx->timeoutTimer->start(options.timeout * 1000);
        }
        return ctx;
    }

    HttpClient *q_ptr;

    QSslConfiguration sslConfiguration = QSslConfiguration::defaultConfiguration();
    std::unordered_map<QNetworkReply *, std::unique_ptr<TaskContext>> tasks;
};

HttpClient::HttpClient(QObject *parent)
    : QNetworkAccessManager(parent), d_ptr(std::make_unique<HttpClientPrivate>(this))
{}

HttpClient::~HttpClient() = default;

QNetworkReply *HttpClient::sendRequest(Method method,
                                       const QUrl &url,
                                       const HttpHeaders &httpHeaders,
                                       const QJsonObject &body,
                                       const HttpRequestOptions &options)
{
    auto request = d_ptr->networkRequest(options.verifyCertificate);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, u"application/json"_s);
    for (const auto &[key, value] : std::as_const(httpHeaders).asKeyValueRange()) {
        request.setRawHeader(key.toUtf8(), value.toUtf8());
    }

    // Q_ENUM 反射：一次转换，复用两次
    const auto methodBytes = HttpClientPrivate::methodToByteArray(method);

    qDebug() << methodBytes << url.toString(QUrl::RemoveUserInfo) << body;

    auto *reply = QNetworkAccessManager::sendCustomRequest(
        request, methodBytes, QJsonDocument(body).toJson(QJsonDocument::Compact));

    // 先完整构建 TaskContext，再 emplace 到 map，避免反向查找
    d_ptr->tasks.emplace(reply, d_ptr->buildTaskContext(options));
    connectReplySignals(reply, options);
    return reply;
}

QJsonObject HttpClient::sync(QNetworkReply *reply)
{
    QPointer<QNetworkReply> replyPtr(reply);
    QJsonObject json;
    QEventLoop loop;
    connect(this, &HttpClient::ready, &loop, [&](QNetworkReply *reply, const QJsonObject &object) {
        if (replyPtr.isNull() || reply == replyPtr) {
            json = object;
            loop.quit();
        }
    });
    loop.exec();
    return json;
}

void HttpClient::cancel(QNetworkReply *reply)
{
    disconnect(reply, nullptr, this, nullptr);
    reply->abort(); // 先中止网络操作（此时 file 仍有效），再释放 TaskContext
    d_ptr->tasks.erase(reply);
    reply->deleteLater();
}

QNetworkReply *
HttpClient::downLoad(const QUrl &url, const QString &filePath, const HttpRequestOptions &options)
{
    Q_ASSERT(!filePath.isEmpty());
    auto file = std::make_unique<QFile>(filePath + u".temp"_s);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qWarning() << u"Cannot open the file:"_s << filePath << file->errorString();
        return nullptr;
    }

    auto request = d_ptr->networkRequest(options.verifyCertificate);
    request.setUrl(url);
    const auto bytes = file->size();
    if (bytes > 0) {
        request.setRawHeader("Range", "bytes="_ba + QByteArray::number(bytes) + "-"_ba);
    }

    qDebug() << u"Download:"_s << url.toString(QUrl::RemoveUserInfo) << u"->"_s << filePath;

    auto *reply = QNetworkAccessManager::get(request);

    auto ctx = d_ptr->buildTaskContext(options);
    ctx->file = std::move(file);
    ctx->downloadPath = filePath;
    ctx->fileBaseSize = bytes;
    d_ptr->tasks.emplace(reply, std::move(ctx));

    // downLoad 的 finished 走 onDownloadFinish（写文件再 queryResult），不能复用 connectReplySignals
    connect(reply, &QNetworkReply::errorOccurred, this, &HttpClient::onErrorOccurred);
    connect(reply, &QNetworkReply::sslErrors, this, &HttpClient::onSslErrors);
    connect(reply, &QNetworkReply::downloadProgress, this, &HttpClient::onDownloadProgress);
    connect(reply, &QNetworkReply::readyRead, this, &HttpClient::onDownloadReadyRead);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onDownloadFinish);
    return reply;
}

QNetworkReply *
HttpClient::upload_put(const QUrl &url, const QString &filePath, const HttpRequestOptions &options)
{
    Q_ASSERT(!filePath.isEmpty());
    auto file = std::make_unique<QFile>(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        qWarning() << u"Cannot open the file:"_s << filePath << file->errorString();
        return nullptr;
    }

    qDebug() << u"Upload:"_s << filePath << u"->"_s << url.toString(QUrl::RemoveUserInfo);

    auto request = d_ptr->networkRequest(options.verifyCertificate);
    request.setUrl(url);
    auto *reply = QNetworkAccessManager::put(request, file.get());

    // 先完整构建 TaskContext（含文件与超时定时器），再 emplace 到 map，避免反向查找
    auto ctx = d_ptr->buildTaskContext(options);
    ctx->file = std::move(file);
    d_ptr->tasks.emplace(reply, std::move(ctx));
    connectReplySignals(reply, options);
    return reply;
}

QNetworkReply *
HttpClient::upload_put(const QUrl &url, const QByteArray &data, const HttpRequestOptions &options)
{
    qDebug() << u"Upload To"_s << url.toString(QUrl::RemoveUserInfo);

    auto request = d_ptr->networkRequest(options.verifyCertificate);
    request.setUrl(url);
    auto *reply = QNetworkAccessManager::put(request, data);

    // 先完整构建 TaskContext，再 emplace 到 map，避免反向查找
    d_ptr->tasks.emplace(reply, d_ptr->buildTaskContext(options));
    connectReplySignals(reply, options);
    return reply;
}

QNetworkReply *
HttpClient::upload_post(const QUrl &url, const QString &filePath, const HttpRequestOptions &options)
{
    Q_ASSERT(!filePath.isEmpty());
    auto file = std::make_unique<QFile>(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        qWarning() << u"Cannot open the file:"_s << filePath << file->errorString();
        return nullptr;
    }

    const auto filename = QFileInfo(filePath).fileName();
    qDebug() << u"Upload:"_s << filePath << u"->"_s
             << url.toString(QUrl::RemoveUserInfo) + u"/"_s + filename;

    // QStringBuilder 链式拼接，避免 .arg() 的临时 QString
    const auto disposition = u"form-data; name=\"file\"; filename=\""_s + filename + u"\""_s;
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, disposition);
    filePart.setBodyDevice(file.get());
    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->append(filePart);

    auto request = d_ptr->networkRequest(options.verifyCertificate);
    request.setUrl(url);

    auto *reply = QNetworkAccessManager::post(request, multiPart);
    multiPart->setParent(reply);

    // 先完整构建 TaskContext（含文件与超时定时器），再 emplace 到 map，避免反向查找
    auto ctx = d_ptr->buildTaskContext(options);
    ctx->file = std::move(file);
    d_ptr->tasks.emplace(reply, std::move(ctx));
    connectReplySignals(reply, options);
    return reply;
}

QNetworkReply *HttpClient::upload_post(const QUrl &url,
                                       const QString &filename,
                                       const QByteArray &data,
                                       const HttpRequestOptions &options)
{
    qDebug() << u"Upload To"_s << url.toString(QUrl::RemoveUserInfo) + u"/"_s + filename;
    const auto disposition = u"form-data; name=\"file\"; filename=\""_s + filename + u"\""_s;
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, disposition);
    filePart.setBody(data);
    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->append(filePart);

    auto request = d_ptr->networkRequest(options.verifyCertificate);
    request.setUrl(url);

    auto *reply = QNetworkAccessManager::post(request, multiPart);
    multiPart->setParent(reply);

    d_ptr->tasks.emplace(reply, d_ptr->buildTaskContext(options));
    connectReplySignals(reply, options);
    return reply;
}

void HttpClient::onReplyFinish()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    const auto object = HttpClientPrivate::jsonFromBytes(reply->readAll());
    queryResult(reply, object);
}

void HttpClient::onErrorOccurred(QNetworkReply::NetworkError code)
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (code == QNetworkReply::NoError || !reply) {
        return;
    }
    qWarning() << u"Network Error :"_s << reply->error() << reply->errorString()
               << reply->readAll();

    QJsonObject object;
    object.insert(u"error"_s, code);
    queryResult(reply, object);
}

void HttpClient::onSslErrors(const QList<QSslError> &errors)
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (errors.isEmpty() || !reply) {
        return;
    }
    qWarning() << u"SSL Errors: "_s;
    for (const auto &error : std::as_const(errors)) {
        qWarning() << error.error() << error.errorString();
    }
    qWarning() << reply->readAll();

    QJsonObject object;
    object.insert(u"error"_s, errors.first().error());
    queryResult(reply, object);
}

void HttpClient::onNetworkTimeout()
{
    auto *timer = qobject_cast<QTimer *>(sender());
    if (!timer) {
        return;
    }

    // 用 ranges::find_if 替代手写循环，按 timer 定位所属 reply
    const auto it = std::ranges::find_if(d_ptr->tasks, [timer](const auto &pair) {
        return pair.second->timeoutTimer.get() == timer;
    });
    if (it == d_ptr->tasks.end()) {
        return;
    }
    auto *targetReply = it->first;

    qWarning() << u"Network Timeout"_s;
    emit timeOut();

    QJsonObject object;
    object.insert(u"error"_s, HttpRequestOptions::NETWORK_TIMEOUT_ERROR);
    queryResult(targetReply, object);
}

void HttpClient::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    auto it = d_ptr->tasks.find(reply);
    if (it == d_ptr->tasks.end()) {
        return;
    }
    const auto &ctx = it->second;
    if (ctx->progressCallback) {
        ctx->progressCallback(bytesReceived + ctx->fileBaseSize, bytesTotal + ctx->fileBaseSize);
    }
}

void HttpClient::onDownloadReadyRead()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    auto it = d_ptr->tasks.find(reply);
    if (it == d_ptr->tasks.end()) {
        return;
    }
    const auto &ctx = it->second;
    if (ctx->file) {
        ctx->file->write(reply->readAll());
    }
}

void HttpClient::onDownloadFinish()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    auto it = d_ptr->tasks.find(reply);
    if (it != d_ptr->tasks.end()) {
        it->second->finalizeDownload(reply);
    }
    queryResult(reply, {});
}

void HttpClient::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    auto it = d_ptr->tasks.find(reply);
    if (it == d_ptr->tasks.end()) {
        return;
    }
    const auto &ctx = it->second;
    if (ctx->progressCallback) {
        ctx->progressCallback(bytesSent, bytesTotal);
    }
}

void HttpClient::connectReplySignals(QNetworkReply *reply, const HttpRequestOptions &options)
{
    connect(reply, &QNetworkReply::errorOccurred, this, &HttpClient::onErrorOccurred);
    connect(reply, &QNetworkReply::sslErrors, this, &HttpClient::onSslErrors);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onReplyFinish);
    if (options.progressCallback) {
        connect(reply, &QNetworkReply::uploadProgress, this, &HttpClient::onUploadProgress);
    }
}

void HttpClient::queryResult(QNetworkReply *reply, const QJsonObject &object)
{
    disconnect(reply, nullptr, this, nullptr);

    auto it = d_ptr->tasks.find(reply);
    if (it == d_ptr->tasks.end()) {
        return;
    }
    auto ctx = std::move(it->second);
    d_ptr->tasks.erase(it);

    auto json = ctx->resultHook ? ctx->resultHook(object) : object;
    if (ctx->callback) {
        ctx->callback(json);
    }
    emit ready(reply, json);
}
