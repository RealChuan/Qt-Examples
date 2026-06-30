#pragma once

#include <LifecycleCallback/lifecyclecallback.hpp>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <functional>
#include <memory>

// 同步等待 / 超时返回的错误码（QJsonValue 中的 "error" 字段）
inline constexpr int NETWORK_TIMEOUT_ERROR = -1;

class HttpClient : public QNetworkAccessManager
{
    Q_OBJECT

public:
    using JsonCallback = LifecycleCallback<const QJsonObject &>;
    using ProgressCallback = LifecycleCallback<qint64, qint64>;
    using HttpHeaders = QHash<QString, QString>;
    using ResultHook = std::function<QJsonObject(const QJsonObject &)>;
    enum class Method : int { GET, POST, PUT, DELETE };
    Q_ENUM(Method)

    // 通用请求选项：超时、证书校验、完成回调、进度回调、结果钩子
    struct RequestOptions
    {
        int timeout = -1;
        bool verifyCertificate = true;
        JsonCallback callback = {};
        ProgressCallback progressCallback = {};
        ResultHook resultHook = {};
    };

    explicit HttpClient(QObject *parent = nullptr);
    ~HttpClient() override;

    QNetworkReply *sendRequest(Method method,
                               const QUrl &url,
                               const HttpHeaders &httpHeaders,
                               const QJsonObject &body,
                               RequestOptions options = {});

    QJsonObject sync(QNetworkReply *reply);
    void cancel(QNetworkReply *reply);

    QNetworkReply *downLoad(const QUrl &url, const QString &filePath, RequestOptions options = {});

    QNetworkReply *
    upload_put(const QUrl &url, const QString &filePath, RequestOptions options = {});
    QNetworkReply *upload_put(const QUrl &url, const QByteArray &data, RequestOptions options = {});
    QNetworkReply *
    upload_post(const QUrl &url, const QString &filePath, RequestOptions options = {});
    QNetworkReply *upload_post(const QUrl &url,
                               const QString &filename,
                               const QByteArray &data,
                               RequestOptions options = {});

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
    void connectReplySignals(QNetworkReply *reply, const RequestOptions &options);
    void queryResult(QNetworkReply *reply, const QJsonObject &object);

    class HttpClientPrivate;
    std::unique_ptr<HttpClientPrivate> d_ptr;
};
