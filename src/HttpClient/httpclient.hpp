#pragma once

#include <LifecycleCallback/lifecyclecallback.hpp>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <functional>
#include <memory>

struct HttpRequestOptions
{
    static constexpr int NETWORK_TIMEOUT_ERROR = -1;

    using JsonCallback = LifecycleCallback<const QJsonObject &>;
    using ProgressCallback = LifecycleCallback<qint64, qint64>;
    using ResultHook = std::function<QJsonObject(const QJsonObject &)>;

    int timeout = -1;
    bool verifyCertificate = true;
    JsonCallback callback = {};
    ProgressCallback progressCallback = {};
    ResultHook resultHook = {};
};

class HttpClient : public QNetworkAccessManager
{
    Q_OBJECT

public:
    using HttpHeaders = QHash<QString, QString>;
    enum class Method : int { GET, POST, PUT, DELETE };
    Q_ENUM(Method)

    explicit HttpClient(QObject *parent = nullptr);
    ~HttpClient() override;

    QNetworkReply *sendRequest(Method method,
                               const QUrl &url,
                               const HttpHeaders &httpHeaders,
                               const QJsonObject &body,
                               const HttpRequestOptions &options = {});

    QJsonObject sync(QNetworkReply *reply);
    void cancel(QNetworkReply *reply);

    QNetworkReply *
    downLoad(const QUrl &url, const QString &filePath, const HttpRequestOptions &options = {});

    QNetworkReply *
    upload_put(const QUrl &url, const QString &filePath, const HttpRequestOptions &options = {});
    QNetworkReply *
    upload_put(const QUrl &url, const QByteArray &data, const HttpRequestOptions &options = {});
    QNetworkReply *
    upload_post(const QUrl &url, const QString &filePath, const HttpRequestOptions &options = {});
    QNetworkReply *upload_post(const QUrl &url,
                               const QString &filename,
                               const QByteArray &data,
                               const HttpRequestOptions &options = {});

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
    void onUploadProgress(qint64 bytesSent, qint64 bytesTotal);

private:
    void connectReplySignals(QNetworkReply *reply, const HttpRequestOptions &options);
    void queryResult(QNetworkReply *reply, const QJsonObject &object);

    class HttpClientPrivate;
    std::unique_ptr<HttpClientPrivate> d_ptr;
};
