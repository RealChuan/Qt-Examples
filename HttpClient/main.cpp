#include "httpclient.hpp"

#include <QCoreApplication>
#include <QJsonObject>

auto main(int argc, char *argv[]) -> int
{
    QCoreApplication a(argc, argv);

    QMetaObject::invokeMethod(
        &a,
        [] {
            HttpClient httpClient;
            auto *reply = httpClient.sendRequest(HttpClient::Method::GET,
                                                 QUrl("http://www.baidu.com"),
                                                 {},
                                                 {},
                                                 30);
            httpClient.sync(reply);
            qApp->quit();
        },
        Qt::QueuedConnection);

    return a.exec();
}
