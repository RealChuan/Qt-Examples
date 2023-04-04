#include "httpclient.h"

#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QUrlQuery>

enum Method { GET, POST, PUT, DELETE };

class HttpClient::HttpClientPrivate
{
public:
    explicit HttpClientPrivate(QObject *parent)
        : q_ptr(parent)
    {
        manager = new QNetworkAccessManager(q_ptr);
    }

    QObject *q_ptr;

    QNetworkAccessManager *manager;
    HttpClient::HttpHeaders headers;
    QString url;
    QUrlQuery params;
    QString charset = "UTF-8";
    Method method = GET;
    QString json;
    bool useJson = false;
};

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new HttpClientPrivate(this))
{}

HttpClient::~HttpClient() = default;

void HttpClient::setHeaders(const HttpClient::HttpHeaders &headers)
{
    d_ptr->headers = headers;
}

void HttpClient::setJson(const QString &json)
{
    d_ptr->json = json;
}

void HttpClient::setParams(const HttpClient::HttpParams &params)
{
    for (HttpParams::const_iterator iter = params.begin(); iter != params.end(); iter++) {
        d_ptr->params.addQueryItem(iter.key(), iter.value().toString());
    }
}

void HttpClient::get(const QString &url)
{
    d_ptr->method = GET;
    startRequest(url);
}

void HttpClient::post(const QString &url)
{
    d_ptr->method = POST;
    startRequest(url);
}

void HttpClient::put(const QString &url)
{
    d_ptr->method = PUT;
    startRequest(url);
}

void HttpClient::remove(const QString &url)
{
    d_ptr->method = DELETE;
    startRequest(url);
}

void HttpClient::download(const QString &url, const QString &savePath)
{
    if (url.isEmpty() || !QUrl(url).isValid()) {
        return;
    }
    d_ptr->url = url;

    if (QFileInfo::exists(savePath)) {
        qWarning() << tr("[Error] File already exists: %1.").arg(savePath);
        return;
    }

    QFile *file = new QFile(savePath, this);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file->close();
        qWarning() << tr("[Error] Open File Error %1: %2").arg(savePath, file->errorString());
        return;
    }
    QNetworkRequest request = createRequest();
    QNetworkReply *reply = d_ptr->manager->get(request);
    if (!reply) {
        return;
    }

    connect(reply, &QNetworkReply::readyRead, this, [=] {
        if (reply->bytesAvailable() > 0) {
            file->write(reply->readAll());
        }
    });

    connect(reply, &QNetworkReply::finished, this, [=] {
        if (reply->bytesAvailable() > 0) {
            file->write(reply->readAll());
        }
        delete file;
        reply->deleteLater();
        emit finish();
    });

    connect(reply, &QNetworkReply::downloadProgress, this, &HttpClient::downloadProgress);
    connect(reply, &QNetworkReply::errorOccurred, this, &HttpClient::slotError);
    connect(reply, &QNetworkReply::sslErrors, this, &HttpClient::slotSslErrors);
}

void HttpClient::upload(const QString &url, const QString &path)
{
    upload(url, QStringList() << path);
}

void HttpClient::upload(const QString &url, const QStringList &paths)
{
    if (paths.isEmpty()) {
        return;
    }
    QHttpMultiPart *multiPart = initMultiPart(url);

    QString inputName = paths.size() == 1 ? "file"
                                          : "files"; // 一个文件时为 file，多个文件时为 files

    for (const QString &path : paths) {
        if (path.isEmpty())
            continue;
        QFile *file = new QFile(path, multiPart);

        if (!file->open(QIODevice::ReadOnly)) {
            qWarning() << tr("[Error] Open File Error %1: %2").arg(path, file->errorString());
            delete multiPart;
            return;
        }

        // 单个文件时，name 为服务器端获取文件的参数名，为 file
        // 多个文件时，name 为服务器端获取文件的参数名，为 files
        // 注意: 服务器是 Java 的则用 form-data
        // 注意: 服务器是 PHP  的则用 multipart/form-data
        QString disposition = QString("form-data; name=\"%1\"; filename=\"%2\"")
                                  .arg(inputName, file->fileName());
        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(disposition));
        filePart.setBodyDevice(file);
        multiPart->append(filePart);
    }

    execUpload(multiPart);
}

void HttpClient::upload(const QString &url, const QByteArray &data)
{
    if (data.isEmpty()) {
        return;
    }

    QHttpMultiPart *multiPart = initMultiPart(url);

    QString disposition = QString("form-data; name=\"file\"; filename=\"no-name\"");
    QHttpPart dataPart;
    dataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(disposition));
    dataPart.setBody(data);
    multiPart->append(dataPart);

    execUpload(multiPart);
}

void HttpClient::slotReadyRead()
{
    qDebug() << "slotReadyRead";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    if (reply->bytesAvailable() <= 0) {
        return;
    }

    QByteArray buf = reply->readAll();
    qDebug() << buf.size();
    emit readyReady(buf);
}

void HttpClient::slotReplyFinish()
{
    qDebug() << "slotReplyFinish";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    if (reply->bytesAvailable() > 0) {
        emit readyReady(reply->readAll());
    }

    reply->deleteLater();
    reply = nullptr;

    emit finish();
}

void HttpClient::slotError(QNetworkReply::NetworkError replyError)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    if (replyError == QNetworkReply::NoError) {
        return;
    }

    emit error(reply->errorString());
}

void HttpClient::slotSslErrors(const QList<QSslError> &errors)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    qDebug() << errors;
}

auto HttpClient::initMultiPart(const QString &url) -> QHttpMultiPart *
{
    if (url.isEmpty() || !QUrl(url).isValid()) {
        return nullptr;
    }
    d_ptr->url = url;

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);
    QList<QPair<QString, QString>> paramItems = d_ptr->params.queryItems();
    for (int i = 0; i < paramItems.size(); ++i) {
        QString name = paramItems.at(i).first;
        QString value = paramItems.at(i).second;

        QHttpPart textPart;
        textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QString("form-data; name=\"%1\"").arg(name));
        textPart.setBody(value.toUtf8());
        multiPart->append(textPart);
    }

    return multiPart;
}

void HttpClient::execUpload(QHttpMultiPart *multiPart)
{
    QNetworkRequest request = createRequest();
    QNetworkReply *reply = d_ptr->manager->post(request, multiPart);
    if (!reply) {
        return;
    }
    multiPart->setParent(reply); // delete the multiPart with the reply
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        emit finish();
    });
    connect(reply, &QNetworkReply::uploadProgress, this, &HttpClient::uploadProgress);
    connect(reply, &QNetworkReply::errorOccurred, this, &HttpClient::slotError);
    connect(reply, &QNetworkReply::sslErrors, this, &HttpClient::slotSslErrors);
}

void HttpClient::startRequest(const QString &url)
{
    if (url.isEmpty() || !QUrl(url).isValid()) {
        return;
    }
    d_ptr->url = url;
    execRequest();
}

void HttpClient::execRequest()
{
    QNetworkRequest request = createRequest();
    QNetworkReply *reply = nullptr;

    switch (d_ptr->method) {
    case GET: reply = d_ptr->manager->get(request); break;
    case POST:
        reply = d_ptr->manager->post(request,
                                     d_ptr->useJson
                                         ? d_ptr->json.toUtf8()
                                         : d_ptr->params.toString(QUrl::FullyEncoded).toUtf8());
        break;
    case PUT:
        reply = d_ptr->manager->put(request,
                                    d_ptr->useJson
                                        ? d_ptr->json.toUtf8()
                                        : d_ptr->params.toString(QUrl::FullyEncoded).toUtf8());
        break;
    case DELETE: reply = d_ptr->manager->deleteResource(request); break;
    default: return;
    }

    buildConnect(reply);
}

auto HttpClient::createRequest() -> QNetworkRequest
{
    bool get = d_ptr->method == GET;
    bool withForm = !get && !d_ptr->useJson; // PUT、POST 或者 DELETE 请求，且 useJson 为 false
    bool withJson = !get && d_ptr->useJson; // PUT、POST 或者 DELETE 请求，且 useJson 为 true

    if (get && !d_ptr->params.isEmpty()) {
        d_ptr->url += "?" + d_ptr->params.toString(QUrl::FullyEncoded);
    }

    qDebug() << tr("URL: ") << d_ptr->url;

    if (withForm) {
        QList<QPair<QString, QString>> paramItems = d_ptr->params.queryItems();
        QString buffer; // 避免多次调用 qDebug() 输入调试信息，每次 qDebug() 都有可能输出行号等

        // 按键值对的方式输出参数
        for (int i = 0; i < paramItems.size(); ++i) {
            QString name = paramItems.at(i).first;
            QString value = paramItems.at(i).second;

            if (0 == i) {
                buffer += QString("params: %1=%2\n").arg(name, value);
            } else {
                buffer += QString("        %1=%2\n").arg(name, value);
            }
        }

        if (!buffer.isEmpty()) {
            qDebug() << buffer;
        }
    }

    // [3] 设置 Content-Type
    // 如果是 POST 请求，useJson 为 true 时添加 Json 的请求头，useJson 为 false 时添加 Form 的请求头
    if (withForm) {
        d_ptr->headers["Content-Type"] = "application/x-www-form-urlencoded";
    } else if (withJson) {
        d_ptr->headers["Content-Type"] = "application/json; charset=utf-8";
    }

    // [4] 添加请求头到 request 中
    QNetworkRequest request(QUrl(d_ptr->url));
    for (auto i = d_ptr->headers.cbegin(); i != d_ptr->headers.cend(); ++i) {
        request.setRawHeader(i.key().toUtf8(), i.value().toUtf8());
    }

    return request;
}

void HttpClient::buildConnect(QNetworkReply *reply)
{
    if (!reply) {
        emit error(tr("reply is nullptr."));
        return;
    }
    connect(reply, &QNetworkReply::readyRead, this, &HttpClient::slotReadyRead);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::slotReplyFinish);
    connect(reply, &QNetworkReply::errorOccurred, this, &HttpClient::slotError);
    connect(reply, &QNetworkReply::sslErrors, this, &HttpClient::slotSslErrors);
}
