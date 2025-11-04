#include "httpclient.hpp"

#include <QCoreApplication>
#include <QFile>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

class HttpClientTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testGetRequest();
    void testPostRequest();
    void testPutRequest();
    void testDeleteRequest();
    void testDownload();
    void testUploadPutFile();
    void testUploadPutData();
    void testUploadPostFile();
    void testUploadPostData();
    void testTimeout();
    void testErrorHandling();
    void testSyncRequest();
    void testCancelRequest();
    void testCustomHeaders();
    void testLifecycleCallbackSafety();
    void testMultipleRequests();

private:
    HttpClient *m_httpClient;
    QString m_baseUrl;
    QTemporaryDir m_tempDir;

    void createFile(const QString &filename, const QByteArray &data);
    void removeFile(const QString &filename);
    void assertFileData(const QString &filename, const QByteArray &data);
    QJsonObject waitForReply(QNetworkReply *reply);
};

void HttpClientTest::initTestCase()
{
    m_httpClient = new HttpClient(this);
    m_baseUrl = "http://127.0.0.1:8000/api";

    // 确保临时目录创建成功
    QVERIFY(m_tempDir.isValid());

    // 给测试服务器启动时间
    QTest::qWait(1000);
}

void HttpClientTest::cleanupTestCase()
{
    delete m_httpClient;
}

void HttpClientTest::createFile(const QString &filename, const QByteArray &data)
{
    QString filepath = m_tempDir.filePath(filename);
    QDir().mkpath(QFileInfo(filepath).absoluteDir().path());

    QFile file(filepath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(data);
    file.close();
}

void HttpClientTest::removeFile(const QString &filename)
{
    QString filepath = m_tempDir.filePath(filename);
    QFile file(filepath);
    if (file.exists()) {
        file.remove();
    }
}

void HttpClientTest::assertFileData(const QString &filename, const QByteArray &data)
{
    QString filepath = m_tempDir.filePath(filename);
    QFile file(filepath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), data);
    file.close();
}

QJsonObject HttpClientTest::waitForReply(QNetworkReply *reply)
{
    QEventLoop loop;
    QJsonObject result;
    bool finished = false;

    // 连接ready信号
    connect(m_httpClient,
            &HttpClient::ready,
            &loop,
            [&](QNetworkReply *readyReply, const QJsonObject &json) {
                if (readyReply == reply) {
                    result = json;
                    finished = true;
                    loop.quit();
                }
            });

    // 设置超时
    QTimer::singleShot(10000, &loop, [&]() {
        if (!finished) {
            qWarning() << "Request timeout";
            loop.quit();
        }
    });

    loop.exec();
    return result;
}

void HttpClientTest::testGetRequest()
{
    auto *reply = m_httpClient
                      ->sendRequest(HttpClient::Method::GET, m_baseUrl + "/test", {}, {}, -1, true);

    QJsonObject response = waitForReply(reply);
    QVERIFY(!response.isEmpty());
    QCOMPARE(response["message"].toString(), QString("GET request successful"));
}

void HttpClientTest::testPostRequest()
{
    QJsonObject requestBody;
    requestBody["test_key"] = "test_value";
    requestBody["number"] = 42;

    auto *reply = m_httpClient->sendRequest(HttpClient::Method::POST,
                                            m_baseUrl + "/test",
                                            {},
                                            requestBody,
                                            -1,
                                            true);

    QJsonObject response = waitForReply(reply);
    QVERIFY(!response.isEmpty());
    QCOMPARE(response["message"].toString(), QString("POST request successful"));

    auto receivedData = response["received_data"].toObject();
    QCOMPARE(receivedData["test_key"].toString(), QString("test_value"));
    QCOMPARE(receivedData["number"].toInt(), 42);
}

void HttpClientTest::testPutRequest()
{
    auto *reply = m_httpClient->upload_put(m_baseUrl + "/upload",
                                           QByteArray("test put data"),
                                           -1,
                                           true);

    QJsonObject response = waitForReply(reply);
    QVERIFY(!response.isEmpty());
    QCOMPARE(response["message"].toString(), QString("PUT upload successful"));
}

void HttpClientTest::testDeleteRequest()
{
    auto *reply = m_httpClient->sendRequest(HttpClient::Method::DELETE,
                                            m_baseUrl + "/test",
                                            {},
                                            {},
                                            -1,
                                            true);

    QJsonObject response = waitForReply(reply);
    QVERIFY(!response.isEmpty());
    QCOMPARE(response["message"].toString(), QString("DELETE request successful"));
}

void HttpClientTest::testDownload()
{
    QString downloadPath = m_tempDir.filePath("test_download.txt");

    // 测试进度回调
    qint64 lastBytesReceived = 0;
    qint64 lastBytesTotal = 0;
    bool progressCalled = false;

    HttpClient::ProgressCallback progressCallback = ([&](qint64 bytesReceived, qint64 bytesTotal) {
        progressCalled = true;
        lastBytesReceived = bytesReceived;
        lastBytesTotal = bytesTotal;
        qDebug() << "Download progress:" << bytesReceived << "/" << bytesTotal;
    });

    auto *reply = m_httpClient->downLoad(QUrl("http://127.0.0.1:8000/download"),
                                         downloadPath,
                                         -1,
                                         true,
                                         progressCallback);

    QJsonObject response = waitForReply(reply);

    // 验证文件下载成功
    QVERIFY(QFile::exists(downloadPath));

    QFile file(downloadPath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), QByteArray("This is test file content for download"));
    file.close();

    // 进度回调应该被调用过（除非文件太小）
    QVERIFY(progressCalled || lastBytesTotal > 0);
}

void HttpClientTest::testUploadPutFile()
{
    QString filename = "test_upload_put.txt";
    QByteArray fileData = "This is test content for PUT file upload";

    createFile(filename, fileData);

    auto *reply = m_httpClient->upload_put(m_baseUrl + "/upload",
                                           m_tempDir.filePath(filename),
                                           -1,
                                           true);

    QJsonObject response = waitForReply(reply);
    QVERIFY(!response.isEmpty());
    QCOMPARE(response["message"].toString(), QString("PUT upload successful"));

    removeFile(filename);
}

void HttpClientTest::testUploadPutData()
{
    QByteArray data = "This is test data for PUT upload";

    auto *reply = m_httpClient->upload_put(m_baseUrl + "/upload", data, -1, true);

    QJsonObject response = waitForReply(reply);
    QVERIFY(!response.isEmpty());
    QCOMPARE(response["message"].toString(), QString("PUT upload successful"));
}

void HttpClientTest::testUploadPostFile()
{
    QString filename = "test_upload_post.txt";
    QByteArray fileData = "This is test content for POST file upload";

    createFile(filename, fileData);

    auto *reply = m_httpClient->upload_post(m_baseUrl + "/upload",
                                            m_tempDir.filePath(filename),
                                            -1,
                                            true);

    QJsonObject response = waitForReply(reply);
    QVERIFY(!response.isEmpty());
    QVERIFY(response["message"].toString().contains("successful"));

    removeFile(filename);
}

void HttpClientTest::testUploadPostData()
{
    QByteArray data = "This is test data for POST upload";

    auto *reply = m_httpClient->upload_post(m_baseUrl + "/upload", "test_file.txt", data, -1, true);

    QJsonObject response = waitForReply(reply);
    QVERIFY(!response.isEmpty());
    QVERIFY(response["message"].toString().contains("successful"));
}

void HttpClientTest::testTimeout()
{
    bool timeoutSignalReceived = false;
    connect(m_httpClient, &HttpClient::timeOut, this, [&]() { timeoutSignalReceived = true; });

    // 使用1秒超时，服务器会睡眠3秒
    auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                            QUrl("http://127.0.0.1:8000/api/timeout"),
                                            {},
                                            {},
                                            1,
                                            true);

    QJsonObject response = waitForReply(reply);

    // 应该收到超时信号
    QVERIFY(timeoutSignalReceived);

    // 响应应该包含错误信息
    QVERIFY(response.contains("error"));
}

void HttpClientTest::testErrorHandling()
{
    // 请求不存在的端点
    auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                            m_baseUrl + "/nonexistent",
                                            {},
                                            {},
                                            -1,
                                            true);

    QJsonObject response = waitForReply(reply);

    // 应该包含错误信息
    QVERIFY(response.contains("error"));
}

void HttpClientTest::testSyncRequest()
{
    auto *reply = m_httpClient
                      ->sendRequest(HttpClient::Method::GET, m_baseUrl + "/test", {}, {}, -1, true);

    QJsonObject response = m_httpClient->sync(reply);
    QVERIFY(!response.isEmpty());
    QCOMPARE(response["message"].toString(), QString("GET request successful"));
}

void HttpClientTest::testCancelRequest()
{
    QEventLoop loop;
    bool callbackCalled = false;

    HttpClient::JsonCallback callback = ([&](const QJsonObject &json) {
        Q_UNUSED(json);
        callbackCalled = true;
        loop.quit();
    });

    auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                            m_baseUrl + "/test",
                                            {},
                                            {},
                                            -1,
                                            true,
                                            callback);

    // 立即取消请求
    m_httpClient->cancel(reply);

    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    loop.exec();

    // 回调不应该被调用，因为请求被取消了
    QVERIFY(!callbackCalled);
}

void HttpClientTest::testCustomHeaders()
{
    HttpClient::HttpHeaders headers;
    headers["X-Custom-Header"] = "CustomValue";
    headers["Authorization"] = "Bearer test-token";

    auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                            QUrl("http://127.0.0.1:8000/api/headers"),
                                            headers,
                                            {},
                                            -1,
                                            true);

    QJsonObject response = waitForReply(reply);
    QVERIFY(!response.isEmpty());
    QVERIFY(response.contains("headers"));
}

void HttpClientTest::testLifecycleCallbackSafety()
{
    class TempObject : public QObject
    {
    public:
        explicit TempObject(QObject *parent)
            : QObject(parent)
        {}

        void callback(const QJsonObject &json)
        {
            Q_UNUSED(json);
            QFAIL("This should not be called");
        }
    };

    // 测试QObject生命周期安全
    auto *tempObject = new TempObject(this);

    auto callback = makeLifecycleCallback(tempObject, &TempObject::callback);

    // 立即删除对象
    delete tempObject;

    // 回调应该无效
    QVERIFY(!callback.isValid());

    auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                            m_baseUrl + "/test",
                                            {},
                                            {},
                                            -1,
                                            true,
                                            callback);

    QJsonObject response = waitForReply(reply);

    // 回调不应该被调用，但请求应该正常完成
    QVERIFY(!response.isEmpty());
}

void HttpClientTest::testMultipleRequests()
{
    const int totalRequests = 3;
    QVector<QJsonObject> responses;
    QEventLoop loop;
    std::atomic<int> completedRequests{0};

    auto sharedCallback = LifecycleCallback<const QJsonObject &>([&](const QJsonObject &json) {
        responses.append(json);
        completedRequests++;

        qDebug() << "Request" << completedRequests << "completed, total:" << totalRequests;

        // 当所有请求都完成时退出事件循环
        if (completedRequests == totalRequests) {
            loop.quit();
        }
    });

    for (int i = 0; i < totalRequests; ++i) {
        auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                                QUrl(m_baseUrl + "/test"),
                                                {},
                                                {},
                                                30,
                                                true,
                                                sharedCallback);
        QVERIFY(reply != nullptr);
    }

    // 设置超时
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
        qWarning() << "Timeout: Only" << completedRequests << "out of" << totalRequests
                   << "requests completed";
        loop.quit();
    });
    timeoutTimer.start(30000);

    // 等待所有请求完成
    loop.exec();

    // 验证结果
    if (completedRequests < totalRequests) {
        QFAIL(QString("Only %1 out of %2 requests completed")
                  .arg(QString::number(completedRequests), QString::number(totalRequests))
                  .toUtf8());
    }

    QCOMPARE(responses.size(), totalRequests);
    for (const auto &response : responses) {
        QVERIFY(!response.isEmpty());
        QCOMPARE(response["message"].toString(), QString("GET request successful"));
    }
}

QTEST_MAIN(HttpClientTest)

#include "httpclient_unittest.moc"
