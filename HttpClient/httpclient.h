#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QNetworkReply>
#include <QObject>

class HttpClientPrivate;
class HttpClient : public QObject
{
    Q_OBJECT
public:
    typedef QHash<QString, QString> HttpHeaders;
    typedef QMap<QString, QVariant> HttpParams;

    explicit HttpClient(QObject *parent = nullptr);
    ~HttpClient();

public slots:
    void setHeaders(const HttpHeaders&);
    void setJson(const QString& json);
    void setParams(const HttpParams& params);

    void get(const QString &url);
    void post(const QString &url);
    void put(const QString &url);
    void remove(const QString &url);
    void download(const QString &url, const QString &savePath);

    // 文件路径
    void upload(const QString &url, const QString &path);
    void upload(const QString &url, const QStringList &paths);

    // 数据
    void upload(const QString &url, const QByteArray &data);

signals:
    void error(const QString&);
    void readyReady(const QByteArray&);
    void downloadProgress(qint64 bytesSent, qint64 bytesTotal);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void finish();

private slots:
    void slotReadyRead();
    void slotReplyFinish();
    void slotError(QNetworkReply::NetworkError);
    void slotSslErrors(const QList<QSslError> &errors);

private:
    QHttpMultiPart *initMultiPart(const QString &url);
    void execUpload(QHttpMultiPart*);
    void startRequest(const QString &url);
    void execRequest();
    QNetworkRequest createRequest();
    void buildConnect(QNetworkReply *reply);

    QScopedPointer<HttpClientPrivate> d;
};

#endif // HTTPCLIENT_H
