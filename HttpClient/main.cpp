#include "httpclient.hpp"

#include <QCoreApplication>
#include <QFile>
#include <QJsonObject>
#include <QTest>

void createFile(const QString &filename, const QByteArray &data)
{
    auto filepath = QString("%1/%2").arg(qApp->applicationDirPath(), filename);
    QDir().mkpath(QFileInfo(filepath).absoluteDir().path());
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << QString("Cannot open the file: %1!").arg(filepath) << file.errorString();
        return;
    }
    file.write(data);
    file.close();
}

void removeFile(const QString &filename)
{
    auto filepath = QString("%1/%2").arg(qApp->applicationDirPath(), filename);
    QFile file(filepath);
    if (file.exists()) {
        file.remove();
    }
}

void assertFileData(const QString &filename, const QByteArray &data)
{
    auto filepath = QString("%1/%2").arg(qApp->applicationDirPath(), filename);
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        QVERIFY2(false, QString("Cannot open the file: %1!").arg(filepath).toStdString().c_str());
        return;
    }
    auto fileData = file.readAll();
    file.close();
    QCOMPARE(fileData, data);
}

void test_get()
{
    HttpClient httpClient;
    auto *reply = httpClient.sendRequest(HttpClient::Method::GET,
                                         QUrl("http://www.baidu.com"),
                                         {},
                                         {},
                                         30);
    httpClient.sync(reply);
}

void test_download(const QString &filename)
{
    auto filepath = QString("%1/%2").arg(qApp->applicationDirPath(), filename);
    QDir().mkpath(QFileInfo(filepath).absoluteDir().path());
    auto url = QString("http://127.0.0.1:8080/files/%1").arg(filename);
    HttpClient httpClient;
    auto *reply = httpClient.downLoad(QUrl(url), filepath, 30);
    httpClient.sync(reply);
}

void test_upload_file(const QString &filename)
{
    auto filepath = QString("%1/%2").arg(qApp->applicationDirPath(), filename);
    auto url = QString("http://127.0.0.1:8080/files/%1").arg(filename);
    HttpClient httpClient;
    auto *reply = httpClient.upload_put(QUrl(url), filepath, 30);
    httpClient.sync(reply);
}

void test_upload_data(const QString &filename, const QByteArray &data)
{
    auto filepath = QString("%1/%2").arg(qApp->applicationDirPath(), filename);
    auto url = QString("http://127.0.0.1:8080/files/%1").arg(filename);
    HttpClient httpClient;
    auto *reply = httpClient.upload_put(QUrl(url), data, 30);
    httpClient.sync(reply);
}

void test_upload_post(const QString &path, const QString &filename)
{
    auto filepath = QString("%1/%2").arg(qApp->applicationDirPath(), filename);
    auto url = QString("http://127.0.0.1:8080/files");
    if (!path.isEmpty()) {
        url += QString("/%1").arg(path);
    }
    HttpClient httpClient;
    auto *reply = httpClient.upload_post(QUrl(url), filepath, 30);
    httpClient.sync(reply);
}

void test_upload_post(const QString &path, const QString &filename, const QByteArray &data)
{
    auto filepath = QString("%1/%2").arg(qApp->applicationDirPath(), filename);
    auto url = QString("http://127.0.0.1:8080/files");
    if (!path.isEmpty()) {
        url += QString("/%1").arg(path);
    }
    HttpClient httpClient;
    auto *reply = httpClient.upload_post(QUrl(url), QFileInfo(filepath).fileName(), data, 30);
    httpClient.sync(reply);
}

void test_delete(const QString &filename)
{
    auto url = QString("http://127.0.0.1:8080/files/%1").arg(filename);
    HttpClient httpClient;
    auto *reply = httpClient.sendRequest(HttpClient::Method::DELETE, QUrl(url), {}, {}, 30);
    httpClient.sync(reply);
}

class HttpClientTest : public QObject
{
    Q_OBJECT

    QStringList paths = {"", "qt", "qt/1", "qt/1/2", "qt/1/2/3"};
private slots:
    void onTestGet() { test_get(); }

    void onTestUploadPutFile()
    {
        auto filename = "upload_put_file.txt";
        auto data = "upload put file data";
        for (const auto &path : std::as_const(paths)) {
            QString filepath = filename;
            QByteArray filedata = data;
            if (!path.isEmpty()) {
                filepath = path + "/" + filename;
                filedata += QByteArray(" ") + path.toUtf8();
            }

            test_delete(filepath);

            createFile(filepath, filedata);
            test_upload_file(filepath);

            removeFile(filepath);
            test_download(filepath);
            assertFileData(filepath, filedata);
        }
    }

    void onTestUploadPutData()
    {
        auto filename = "upload_put_data.txt";
        auto data = "upload put data";
        for (const auto &path : std::as_const(paths)) {
            QString filepath = filename;
            QByteArray filedata = data;
            if (!path.isEmpty()) {
                filepath = path + "/" + filename;
                filedata += QByteArray(" ") + path.toUtf8();
            }

            test_delete(filepath);

            test_upload_data(filepath, filedata);

            test_download(filepath);
            assertFileData(filepath, filedata);
        }
    }

    void onTestUploadPost()
    {
        auto filename = "upload_post_file.txt";
        auto data = "upload post file data";
        for (const auto &path : std::as_const(paths)) {
            QString filepath = filename;
            QByteArray filedata = data;
            if (!path.isEmpty()) {
                filepath = path + "/" + filename;
                filedata += QByteArray(" ") + path.toUtf8();
            }

            test_delete(filepath);

            createFile(filepath, filedata);
            test_upload_post(path, filepath);

            removeFile(filepath);
            test_download(filepath);
            assertFileData(filepath, filedata);
        }
    }

    void onTestUploadPostData()
    {
        auto filename = "upload_post_data.txt";
        auto data = "upload post data";
        for (const auto &path : std::as_const(paths)) {
            QString filepath = filename;
            QByteArray filedata = data;
            if (!path.isEmpty()) {
                filepath = path + "/" + filename;
                filedata += QByteArray(" ") + path.toUtf8();
            }

            test_delete(filepath);

            test_upload_post(path, filepath, filedata);

            test_download(filepath);
            assertFileData(filepath, filedata);
        }
    }
};

QTEST_MAIN(HttpClientTest)

#include "main.moc"