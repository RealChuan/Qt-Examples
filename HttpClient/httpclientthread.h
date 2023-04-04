#ifndef HTTPCLIENTTHREAD_H
#define HTTPCLIENTTHREAD_H

#include <QThread>

#include "httpclient.h"

class HttpClientThread : public QThread
{
    Q_OBJECT
public:
    explicit HttpClientThread(QObject *parent = nullptr);
    ~HttpClientThread() override;

signals:
    void setHeaders(const HttpClient::HttpHeaders &);
    void setJson(const QString &json);
    void setParams(const HttpClient::HttpParams &params);

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

    //-------------------------------------------------------------
    void error(const QString &);
    void readyReady(const QByteArray &);
    void downloadProgress(qint64 bytesSent, qint64 bytesTotal);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void finish();

protected:
    void run() override;
};

#endif // HTTPCLIENTTHREAD_H
