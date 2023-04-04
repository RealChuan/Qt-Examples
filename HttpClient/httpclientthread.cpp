#include "httpclientthread.h"

HttpClientThread::HttpClientThread(QObject *parent)
    : QThread(parent)
{
    start();
}

HttpClientThread::~HttpClientThread()
{
    if (isRunning()) {
        quit();
        wait();
    }
}

void HttpClientThread::run()
{
    QScopedPointer<HttpClient> httpClient(new HttpClient);
    connect(this, &HttpClientThread::setHeaders, httpClient.data(), &HttpClient::setHeaders);
    connect(this, &HttpClientThread::setJson, httpClient.data(), &HttpClient::setJson);
    connect(this, &HttpClientThread::setParams, httpClient.data(), &HttpClient::setParams);

    connect(this, &HttpClientThread::get, httpClient.data(), &HttpClient::get);
    connect(this, &HttpClientThread::post, httpClient.data(), &HttpClient::post);
    connect(this, &HttpClientThread::put, httpClient.data(), &HttpClient::put);
    connect(this, &HttpClientThread::remove, httpClient.data(), &HttpClient::remove);
    connect(this, &HttpClientThread::download, httpClient.data(), &HttpClient::download);

    //    connect(this, SIGNAL(upload(const QString &url, const QString &path)),
    //            httpClient.data(), SLOT(upload(const QString &url, const QString &path)));
    //    connect(this, SIGNAL(upload(const QString &url, const QStringList &paths)),
    //            httpClient.data(), SLOT(upload(const QString &url, const QStringList &paths)));
    //    connect(this, SIGNAL(upload(const QString &url, const QByteArray &data)),
    //            httpClient.data(), SLOT(upload(const QString &url, const QByteArray &data)));

    connect(httpClient.data(), &HttpClient::error, this, &HttpClientThread::error);
    connect(httpClient.data(), &HttpClient::readyReady, this, &HttpClientThread::readyReady);
    connect(httpClient.data(),
            &HttpClient::downloadProgress,
            this,
            &HttpClientThread::downloadProgress);
    connect(httpClient.data(), &HttpClient::uploadProgress, this, &HttpClientThread::uploadProgress);
    connect(httpClient.data(), &HttpClient::finish, this, &HttpClientThread::finish);

    exec();
}
