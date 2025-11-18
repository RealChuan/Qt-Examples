#include "httpclient.hpp"

#include <QEventLoop>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QTimer>

class HttpClient::HttpClientPrivate
{
public:
    explicit HttpClientPrivate(HttpClient *q)
        : q_ptr(q)
    {
        sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    }

    QNetworkRequest networkRequest(bool verifyCertificate)
    {
        QNetworkRequest request;
        if (!verifyCertificate) {
            request.setSslConfiguration(sslConfiguration);
        }
        return request;
    }

    void clearTask(QNetworkReply *reply)
    {
        tasks.remove(reply);
        auto *download = downloads.take(reply);
        if (download) {
            delete download;
        }
        auto *file = uploads.take(reply);
        if (file) {
            file->deleteLater();
        }
    }

    QByteArray methodToString(Method method) const
    {
        switch (method) {
        case Method::GET: return "GET";
        case Method::POST: return "POST";
        case Method::PUT: return "PUT";
        case Method::DELETE: return "DELETE";
        default: break;
        }
        Q_ASSERT(false);
        return "UNKNOWN";
    }

    auto jsonFromBytes(const QByteArray &bytes) -> QJsonObject
    {
        QJsonParseError jsonParseError;
        auto jsonDocument = QJsonDocument::fromJson(bytes, &jsonParseError);
        if (QJsonParseError::NoError != jsonParseError.error) {
            qWarning() << QString("%1\nOffset: %2")
                              .arg(jsonParseError.errorString(),
                                   QString::number(jsonParseError.offset))
                       << bytes;
            return {};
        }
        return jsonDocument.object();
    }

    struct Download
    {
        ~Download()
        {
            if (!filePtr.isNull()) {
                filePtr->deleteLater();
            }
        }

        QString filePath;
        qint64 fileBaseSize;
        QPointer<QFile> filePtr;
        ProgressCallback progressCallback = {};
    };

    HttpClient *q_ptr;

    QSslConfiguration sslConfiguration = QSslConfiguration::defaultConfiguration();
    QMap<QNetworkReply *, JsonCallback> tasks;
    QMap<QNetworkReply *, Download *> downloads;
    QMap<QNetworkReply *, QFile *> uploads;
};

HttpClient::HttpClient(QObject *parent)
    : QNetworkAccessManager(parent)
    , d_ptr(new HttpClientPrivate(this))
{}

HttpClient::~HttpClient() {}

QNetworkReply *HttpClient::sendRequest(Method method,
                                       const QUrl &url,
                                       const HttpHeaders &httpHeaders,
                                       const QJsonObject &body,
                                       int timeout,
                                       bool verifyCertificate,
                                       JsonCallback callback)
{
    auto request = d_ptr->networkRequest(verifyCertificate);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    for (auto iter = httpHeaders.begin(); iter != httpHeaders.end(); iter++) {
        request.setRawHeader(iter.key().toUtf8(), iter.value().toUtf8());
    }

    qDebug() << d_ptr->methodToString(method) << url.toString(QUrl::RemoveUserInfo) << body;

    auto *reply = QNetworkAccessManager::sendCustomRequest(request,
                                                           d_ptr->methodToString(method),
                                                           QJsonDocument(body).toJson(
                                                               QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onReplyFinish);
    connect(reply, &QNetworkReply::errorOccurred, this, &HttpClient::onErrorOccurred);
    connect(reply, &QNetworkReply::sslErrors, this, &HttpClient::onSslErrors);
    d_ptr->tasks.insert(reply, callback);
    if (timeout > 0) {
        auto *timer = new QTimer(reply);
        connect(timer, &QTimer::timeout, this, &HttpClient::onNetworkTimeout);
        timer->start(timeout * 1000);
    }
    return reply;
}

QJsonObject HttpClient::sync(QNetworkReply *reply)
{
    QPointer<QNetworkReply> replyPtr(reply);
    QJsonObject json;
    QEventLoop loop;
    connect(this, &HttpClient::ready, &loop, [&](QNetworkReply *reply, const QJsonObject &object) {
        if (replyPtr.isNull()) {
            loop.quit();
        }
        if (reply == replyPtr) {
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
    d_ptr->clearTask(reply);
    reply->abort();
    reply->deleteLater();
}

QNetworkReply *HttpClient::downLoad(const QUrl &url,
                                    const QString &filePath,
                                    int timeout,
                                    bool verifyCertificate,
                                    ProgressCallback progressCallback,
                                    JsonCallback callback)
{
    Q_ASSERT(!filePath.isEmpty());
    auto *file = new QFile(filePath + ".temp", this);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qWarning() << QString("Cannot open the file: %1!").arg(filePath) << file->errorString();
        file->deleteLater();
        return nullptr;
    }

    auto request = d_ptr->networkRequest(verifyCertificate);
    request.setUrl(url);
    auto bytes = file->size();
    if (bytes > 0) {
        const QByteArray fromRange = "bytes=" + QByteArray::number(bytes) + "-";
        request.setRawHeader("Range", fromRange);
    }
    qDebug() << QString("Download: %1->%2").arg(url.toString(QUrl::RemoveUserInfo), filePath);

    auto *reply = QNetworkAccessManager::get(request);
    d_ptr->downloads
        .insert(reply, new HttpClientPrivate::Download{filePath, bytes, file, progressCallback});
    d_ptr->tasks.insert(reply, callback);
    connect(reply, &QNetworkReply::errorOccurred, this, &HttpClient::onErrorOccurred);
    connect(reply, &QNetworkReply::sslErrors, this, &HttpClient::onSslErrors);
    connect(reply, &QNetworkReply::downloadProgress, this, &HttpClient::onDownloadProgress);
    connect(reply, &QNetworkReply::readyRead, this, &HttpClient::onDownloadReadyRead);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onDownloadFinish);
    if (timeout > 0) {
        auto *timer = new QTimer(reply);
        connect(timer, &QTimer::timeout, this, &HttpClient::onNetworkTimeout);
        timer->start(timeout * 1000);
    }
    return reply;
}

QNetworkReply *HttpClient::upload_put(const QUrl &url,
                                      const QString &filePath,
                                      int timeout,
                                      bool verifyCertificate,
                                      JsonCallback callback)
{
    Q_ASSERT(!filePath.isEmpty());
    auto *file = new QFile(filePath, this);
    if (!file->open(QIODevice::ReadOnly)) {
        qWarning() << QString("Cannot open the file: %1!").arg(filePath) << file->errorString();
        file->deleteLater();
        return nullptr;
    }
    qDebug() << QString("Upload: %1->%2").arg(filePath, url.toString(QUrl::RemoveUserInfo));

    auto request = d_ptr->networkRequest(verifyCertificate);
    request.setUrl(url);
    auto *reply = QNetworkAccessManager::put(request, file);
    file->setParent(reply);
    d_ptr->uploads.insert(reply, file);
    connectUploadSlots(reply, timeout, callback);
    return reply;
}

QNetworkReply *HttpClient::upload_put(const QUrl &url,
                                      const QByteArray &data,
                                      int timeout,
                                      bool verifyCertificate,
                                      JsonCallback callback)
{
    qDebug() << QString("Upload To %1").arg(url.toString(QUrl::RemoveUserInfo));

    auto request = d_ptr->networkRequest(verifyCertificate);
    request.setUrl(url);
    auto *reply = QNetworkAccessManager::put(request, data);
    connectUploadSlots(reply, timeout, callback);
    return reply;
}

QNetworkReply *HttpClient::upload_post(const QUrl &url,
                                       const QString &filePath,
                                       int timeout,
                                       bool verifyCertificate,
                                       JsonCallback callback)
{
    Q_ASSERT(!filePath.isEmpty());
    auto *file = new QFile(filePath, this);
    if (!file->open(QIODevice::ReadOnly)) {
        qWarning() << QString("Cannot open the file: %1!").arg(filePath) << file->errorString();
        file->deleteLater();
        return nullptr;
    }
    auto filename = QFileInfo(filePath).fileName();
    qDebug() << QString("Upload: %1->%2")
                    .arg(filePath, url.toString(QUrl::RemoveUserInfo) + "/" + filename);

    auto disposition = QString("form-data; name=\"%1\"; filename=\"%2\"").arg("file", filename);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(disposition));
    filePart.setBodyDevice(file);
    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->append(filePart);

    auto request = d_ptr->networkRequest(verifyCertificate);
    request.setUrl(url);

    auto *reply = QNetworkAccessManager::post(request, multiPart);
    file->setParent(reply);
    multiPart->setParent(reply);
    d_ptr->uploads.insert(reply, file);
    connectUploadSlots(reply, timeout, callback);
    return reply;
}

QNetworkReply *HttpClient::upload_post(const QUrl &url,
                                       const QString &filename,
                                       const QByteArray &data,
                                       int timeout,
                                       bool verifyCertificate,
                                       JsonCallback callback)
{
    qDebug() << QString("Upload To %1").arg(url.toString(QUrl::RemoveUserInfo) + "/" + filename);
    auto disposition = QString("form-data; name=\"%1\"; filename=\"%2\"").arg("file", filename);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(disposition));
    filePart.setBody(data);
    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->append(filePart);

    auto request = d_ptr->networkRequest(verifyCertificate);
    request.setUrl(url);

    auto *reply = QNetworkAccessManager::post(request, multiPart);
    multiPart->setParent(reply);
    connectUploadSlots(reply, timeout, callback);
    return reply;
}

void HttpClient::onReplyFinish()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    const auto object(d_ptr->jsonFromBytes(reply->readAll()));
    queryResult(reply, object);
}

void HttpClient::onErrorOccurred(QNetworkReply::NetworkError code)
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (code == QNetworkReply::NoError || !reply) {
        return;
    }
    qWarning() << "Network Error :" << reply->error() << reply->errorString() << reply->readAll();

    QJsonObject object;
    object.insert("error", code);
    queryResult(reply, object);
}

void HttpClient::onSslErrors(const QList<QSslError> &errors)
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (errors.isEmpty() || !reply) {
        return;
    }
    qWarning() << "SSL Errors: ";
    for (const auto &error : std::as_const(errors)) {
        qWarning() << error.error() << error.errorString();
    }
    qWarning() << reply->readAll();

    QJsonObject object;
    object.insert("error", errors.first().error());
    queryResult(reply, object);
}

void HttpClient::onNetworkTimeout()
{
    qWarning() << "Network Timeout";

    emit timeOut();
    auto *timer = qobject_cast<QTimer *>(sender());
    if (!timer) {
        return;
    }
    timer->stop();
    timer->deleteLater();
    auto *reply = qobject_cast<QNetworkReply *>(timer->parent());
    if (!reply) {
        return;
    }

    QJsonObject object;
    object.insert("error", NETWORK_TIMEOUT_ERROR);
    queryResult(reply, object);
}

void HttpClient::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    auto *download = d_ptr->downloads.value(reply);
    if (!download) {
        return;
    }
    auto progressCallback = download->progressCallback;
    if (!progressCallback) {
        return;
    }
    progressCallback(bytesReceived + download->fileBaseSize, bytesTotal + download->fileBaseSize);
}

void HttpClient::onDownloadReadyRead()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    auto *download = d_ptr->downloads.value(reply);
    if (!download) {
        return;
    }
    if (download->filePtr.isNull()) {
        return;
    }
    download->filePtr->write(reply->readAll());
}

void HttpClient::onDownloadFinish()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    auto *download = d_ptr->downloads.value(reply);
    if (download && !download->filePtr.isNull()) {
        download->filePtr->write(reply->readAll());
        download->filePtr->flush();
        download->filePtr->close();
        if (QFile::exists(download->filePath)) {
            if (!QFile::remove(download->filePath)) {
                qWarning() << "Failed to remove existing file:" << download->filePath;
            }
        }
        if (!download->filePtr->rename(download->filePath)) {
            qWarning() << "Failed to rename temp file to:" << download->filePath;
        }
    }
    queryResult(reply, {});
}

QJsonObject HttpClient::hookResult(const QJsonObject &object)
{
    return object;
}

void HttpClient::connectUploadSlots(QNetworkReply *reply, int timeout, JsonCallback callback)
{
    d_ptr->tasks.insert(reply, callback);
    connect(reply, &QNetworkReply::errorOccurred, this, &HttpClient::onErrorOccurred);
    connect(reply, &QNetworkReply::sslErrors, this, &HttpClient::onSslErrors);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onReplyFinish);
    if (timeout > 0) {
        auto *timer = new QTimer(reply);
        connect(timer, &QTimer::timeout, this, &HttpClient::onNetworkTimeout);
        timer->start(timeout * 1000);
    }
}

void HttpClient::queryResult(QNetworkReply *reply, const QJsonObject &object)
{
    qDebug() << object;

    disconnect(reply, nullptr, this, nullptr);

    auto callback = d_ptr->tasks.value(reply);
    d_ptr->clearTask(reply);
    auto json = hookResult(object);
    if (callback) {
        callback(json);
    }
    emit ready(reply, json);
}
