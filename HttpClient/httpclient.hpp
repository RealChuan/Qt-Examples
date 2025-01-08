#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>

#define NETWORK_TIMEOUT_ERROR -1

class HttpClient : public QNetworkAccessManager
{
    Q_OBJECT
public:
    using CallBack = std::function<void(const QJsonObject &)>;
    using ProgressCallBack = std::function<void(qint64 download, qint64 total)>;
    using HttpHeaders = QHash<QString, QString>;
    enum class Method : int { GET, POST, PUT, DELETE };

    explicit HttpClient(QObject *parent = nullptr);
    ~HttpClient() override;

    QNetworkReply *sendRequest(Method method,
                               const QUrl &url,
                               const HttpHeaders &httpHeaders,
                               const QJsonObject &body,
                               int timeout = -1,
                               bool verifyCertificate = true,
                               CallBack callBack = nullptr);

    QJsonObject sync(QNetworkReply *reply);
    void cancel(QNetworkReply *reply);

    QNetworkReply *downLoad(const QUrl &url,
                            const QString &filePath,
                            int timeout = -1,
                            bool verifyCertificate = true,
                            ProgressCallBack progressCallBack = nullptr,
                            CallBack callBack = nullptr);

    QNetworkReply *upload_put(const QUrl &url,
                              const QString &filePath,
                              int timeout = -1,
                              bool verifyCertificate = true,
                              CallBack callBack = nullptr);
    QNetworkReply *upload_put(const QUrl &url,
                              const QByteArray &data,
                              int timeout = -1,
                              bool verifyCertificate = true,
                              CallBack callBack = nullptr);
    QNetworkReply *upload_post(const QUrl &url,
                               const QString &filePath,
                               int timeout = -1,
                               bool verifyCertificate = true,
                               CallBack callBack = nullptr);
    QNetworkReply *upload_post(const QUrl &url,
                               const QString &filename,
                               const QByteArray &data,
                               int timeout = -1,
                               bool verifyCertificate = true,
                               CallBack callBack = nullptr);

signals:
    void timeOut();
    void ready(QNetworkReply *reply, const QJsonObject &object);

private slots:
    void onReplyFinish();
    void onErrorOccurred(QNetworkReply::NetworkError code);
    void onSslErrors(const QList<QSslError> &errors);
    void onNetworkTimeout();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadReadyRead();
    void onDownloadFinish();
    void onUploadFinish();

protected:
    virtual QJsonObject hookResult(const QJsonObject &object);

private:
    void connectUploadSlots(QNetworkReply *reply, int timeout, CallBack callBack);
    void queryResult(QNetworkReply *reply, const QJsonObject &object);

    class HttpClientPrivate;
    QScopedPointer<HttpClientPrivate> d_ptr;
};
