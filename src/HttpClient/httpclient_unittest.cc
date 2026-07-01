#include "httpclient.hpp"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QMutex>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <QTimer>

class HttpClientTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基础功能测试
    void testGetRequest();
    void testPostRequest();
    void testPutRequest();
    void testDeleteRequest();
    void testPostRequestWithVariousData();

    // 文件传输测试
    void testDownload();
    void testDownloadProgress();
    void testUploadPutFile();
    void testUploadPutData();
    void testUploadPostFile();
    void testUploadPostData();
    void testUploadProgress();

    // 错误和边界情况测试
    void testTimeout();
    void testNetworkErrorScenarios();
    void testCancelMechanism();
    void testBoundaryConditions();
    void testDownloadEdgeCases();

    // 功能特性测试
    void testSyncRequest();
    void testCustomHeaders();
    void testMultipleRequests();
    void testConcurrentRequests();
    void testHttpClientStateManagement();

private:
    // 测试辅助方法
    bool isTestServerAvailable();
    QNetworkReply *createBasicGetRequest();
    void verifySuccessfulResponse(const QJsonObject &response);
    void createFile(const QString &filename, const QByteArray &data);
    void removeFile(const QString &filename);
    QJsonObject waitForReply(QNetworkReply *reply);
    void cleanupReply(QNetworkReply *reply);

    HttpClient *m_httpClient;
    QString m_baseUrl;
    QTemporaryDir m_tempDir;
    bool m_serverAvailable;
};

void HttpClientTest::initTestCase()
{
    m_httpClient = new HttpClient(this);
    m_baseUrl = "http://127.0.0.1:8000/api";
    QVERIFY(m_tempDir.isValid());

    qDebug() << "HttpClient Unit Tests Starting";
    qDebug() << "Base URL:" << m_baseUrl;
    qDebug() << "Temp Dir:" << m_tempDir.path();

    m_serverAvailable = isTestServerAvailable();

    if (!m_serverAvailable) {
        qWarning() << "Test server not available at" << m_baseUrl;
        qWarning() << "Some tests will be skipped";
    }
}

void HttpClientTest::cleanupTestCase()
{
    delete m_httpClient;
    qDebug() << "HttpClient Unit Tests Finished";
}

void HttpClientTest::init()
{
    if (!m_serverAvailable) {
        QSKIP("Test server not available");
    }
}

void HttpClientTest::cleanup()
{
    // 每个测试后的小延迟，确保资源清理
    QTest::qWait(100);
}

bool HttpClientTest::isTestServerAvailable()
{
    QNetworkAccessManager nam;
    QNetworkRequest request(QUrl("http://127.0.0.1:8000/api/health"));
    QNetworkReply *reply = nam.get(request);

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timeoutTimer.start(3000);
    loop.exec();

    bool available = (reply->error() == QNetworkReply::NoError);

    if (available) {
        QByteArray response = reply->readAll();
        qDebug() << "Server health check response:" << response;
    }

    reply->deleteLater();
    return available;
}

QNetworkReply *HttpClientTest::createBasicGetRequest()
{ return m_httpClient->sendRequest(HttpClient::Method::GET, m_baseUrl + "/test", {}, {}); }

void HttpClientTest::verifySuccessfulResponse(const QJsonObject &response)
{
    QVERIFY(!response.isEmpty());
    QVERIFY(!response.contains("error"));

    // 根据API响应结构验证基本字段
    if (response.contains("method")) {
        QString method = response["method"].toString();
        QVERIFY(!method.isEmpty());
    }

    if (response.contains("message")) {
        QString message = response["message"].toString();
        QVERIFY(!message.isEmpty());
    }
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
            qWarning() << "Request timeout in waitForReply for URL:" << reply->url().toString();
            loop.quit();
        }
    });

    loop.exec();
    return result;
}

void HttpClientTest::cleanupReply(QNetworkReply *reply)
{
    if (reply) {
        reply->deleteLater();
    }
}

// 基础功能测试
void HttpClientTest::testGetRequest()
{
    auto *reply = createBasicGetRequest();
    QVERIFY(reply != nullptr);

    QJsonObject response = waitForReply(reply);
    verifySuccessfulResponse(response);
    QCOMPARE(response["method"].toString(), QString("GET"));

    QVERIFY(reply->isFinished());
    QCOMPARE(reply->error(), QNetworkReply::NoError);

    cleanupReply(reply);
}

void HttpClientTest::testPostRequest()
{
    QJsonObject requestBody;
    requestBody["test_key"] = "test_value";
    requestBody["number"] = 42;

    auto *reply
        = m_httpClient->sendRequest(HttpClient::Method::POST, m_baseUrl + "/echo", {}, requestBody);

    QJsonObject response = waitForReply(reply);
    verifySuccessfulResponse(response);

    // 验证回显数据
    QVERIFY(response.contains("echo_data"));

    cleanupReply(reply);
}

void HttpClientTest::testPutRequest()
{
    auto *reply = m_httpClient->upload_put(m_baseUrl + "/echo", QByteArray("test put data"));

    QJsonObject response = waitForReply(reply);
    verifySuccessfulResponse(response);

    cleanupReply(reply);
}

void HttpClientTest::testDeleteRequest()
{
    auto *reply
        = m_httpClient->sendRequest(HttpClient::Method::DELETE, m_baseUrl + "/test", {}, {});

    QJsonObject response = waitForReply(reply);
    verifySuccessfulResponse(response);
    QCOMPARE(response["method"].toString(), QString("DELETE"));

    cleanupReply(reply);
}

void HttpClientTest::testPostRequestWithVariousData()
{
    struct TestCase
    {
        QJsonObject requestBody;
        QString description;
    };

    QVector<TestCase> testCases
        = {{QJsonObject{{"string", "value"}}, "Simple string"},
           {QJsonObject{{"number", 42}, {"float", 3.14}}, "Numbers"},
           {QJsonObject{{"bool_true", true}, {"bool_false", false}}, "Booleans"},
           {QJsonObject{{"array", QJsonArray{1, 2, 3}}}, "Array"},
           {QJsonObject{{"nested", QJsonObject{{"key", "value"}}}}, "Nested object"},
           {QJsonObject{}, "Empty object"}};

    for (const auto &testCase : testCases) {
        qDebug() << "Testing POST with:" << testCase.description;

        auto *reply = m_httpClient->sendRequest(
            HttpClient::Method::POST, m_baseUrl + "/echo", {}, testCase.requestBody);

        QJsonObject response = waitForReply(reply);
        QVERIFY(!response.isEmpty());
        QVERIFY(response.contains("echo_data"));

        cleanupReply(reply);
    }
}

// 文件传输测试
void HttpClientTest::testDownload()
{
    QString downloadPath = m_tempDir.filePath("test_download.txt");

    auto *reply = m_httpClient->downLoad(QUrl("http://127.0.0.1:8000/download"), downloadPath);

    QJsonObject response = waitForReply(reply);

    // 验证下载完成
    QVERIFY(QFile::exists(downloadPath));

    QFile file(downloadPath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray content = file.readAll();
    file.close();

    QVERIFY(content.contains("test file content"));

    cleanupReply(reply);
}

void HttpClientTest::testDownloadProgress()
{
    QString downloadPath = m_tempDir.filePath("progress_test.txt");

    QVector<qint64> progressValues;
    QMutex progressMutex;
    bool progressCalled = false;

    HttpRequestOptions::ProgressCallback progressCallback
        = [&](qint64 bytesReceived, qint64 bytesTotal) {
              QMutexLocker locker(&progressMutex);
              progressCalled = true;
              progressValues.append(bytesReceived);

              qDebug() << "Download progress:" << bytesReceived << "/" << bytesTotal;

              QVERIFY(bytesReceived >= 0);
              QVERIFY(bytesTotal >= 0);
              if (bytesTotal > 0) {
                  QVERIFY(bytesReceived <= bytesTotal);
              }
          };

    auto *reply = m_httpClient->downLoad(QUrl("http://127.0.0.1:8000/download"),
                                         downloadPath,
                                         {.progressCallback = progressCallback});

    QJsonObject response = waitForReply(reply);

    // 验证进度回调被调用
    QVERIFY(progressCalled);
    QVERIFY(progressValues.size() > 0);

    // 验证文件下载完成
    QVERIFY(QFile::exists(downloadPath));

    cleanupReply(reply);
}

void HttpClientTest::testUploadPutFile()
{
    QString filename = "test_upload_put.txt";
    QByteArray fileData = "This is test content for PUT file upload";

    createFile(filename, fileData);

    auto *reply = m_httpClient->upload_put(m_baseUrl + "/echo", m_tempDir.filePath(filename));

    QJsonObject response = waitForReply(reply);
    verifySuccessfulResponse(response);

    cleanupReply(reply);
    removeFile(filename);
}

void HttpClientTest::testUploadPutData()
{
    QByteArray data = "This is test data for PUT upload";

    auto *reply = m_httpClient->upload_put(m_baseUrl + "/echo", data);

    QJsonObject response = waitForReply(reply);
    verifySuccessfulResponse(response);

    cleanupReply(reply);
}

void HttpClientTest::testUploadPostFile()
{
    QString filename = "test_upload_post.txt";
    QByteArray fileData = "This is test content for POST file upload";

    createFile(filename, fileData);

    auto *reply = m_httpClient->upload_post(m_baseUrl + "/echo", m_tempDir.filePath(filename));

    QJsonObject response = waitForReply(reply);
    verifySuccessfulResponse(response);

    cleanupReply(reply);
    removeFile(filename);
}

void HttpClientTest::testUploadPostData()
{
    QByteArray data = "This is test data for POST upload";

    auto *reply = m_httpClient->upload_post(m_baseUrl + "/echo", "test_file.txt", data);

    QJsonObject response = waitForReply(reply);
    verifySuccessfulResponse(response);

    cleanupReply(reply);
}

void HttpClientTest::testUploadProgress()
{
    QByteArray data(1024, 'X'); // 1KB 数据，足够触发进度回调

    QVector<qint64> progressValues;
    QMutex progressMutex;
    bool progressCalled = false;

    HttpRequestOptions::ProgressCallback progressCallback
        = [&](qint64 bytesSent, qint64 bytesTotal) {
              QMutexLocker locker(&progressMutex);
              progressCalled = true;
              progressValues.append(bytesSent);

              qDebug() << "Upload progress:" << bytesSent << "/" << bytesTotal;

              QVERIFY(bytesSent >= 0);
              QVERIFY(bytesTotal >= 0);
              if (bytesTotal > 0) {
                  QVERIFY(bytesSent <= bytesTotal);
              }
          };

    auto *reply = m_httpClient->upload_put(
        m_baseUrl + "/echo", data, {.progressCallback = progressCallback});

    QJsonObject response = waitForReply(reply);
    verifySuccessfulResponse(response);

    // 验证进度回调被调用（小数据量可能只触发一次）
    QVERIFY(progressCalled);

    cleanupReply(reply);
}

// 错误和边界情况测试
void HttpClientTest::testTimeout()
{
    bool timeoutReceived = false;
    bool callbackCalled = false;

    connect(m_httpClient, &HttpClient::timeOut, this, [&timeoutReceived]() {
        timeoutReceived = true;
        qDebug() << "Timeout signal received";
    });

    HttpRequestOptions::JsonCallback callback = [&callbackCalled](const QJsonObject &json) {
        callbackCalled = true;
        qDebug() << "Timeout callback called with error:" << json["error"].toInt();
    };

    // 使用1秒超时，请求会超时的端点
    auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                            QUrl("http://127.0.0.1:8000/api/timeout"),
                                            {},
                                            {},
                                            {.timeout = 1, .callback = callback});

    QJsonObject response = waitForReply(reply);

    QVERIFY(timeoutReceived);
    QVERIFY(callbackCalled);
    QVERIFY(response.contains("error"));

    cleanupReply(reply);
}

void HttpClientTest::testNetworkErrorScenarios()
{
    struct ErrorCase
    {
        QUrl url;
        QString description;
        bool expectError;
    };

    QVector<ErrorCase> errorCases
        = {{QUrl("http://invalid-domain-that-definitely-does-not-exist-12345.xyz"),
            "Invalid domain",
            true},
           {QUrl("http://192.168.255.255:9999"), "Unreachable IP", true},
           {m_baseUrl + "/test", "Valid URL", false}};

    for (const auto &errorCase : errorCases) {
        qDebug() << "Testing error case:" << errorCase.description;

        bool errorOccurred = false;
        bool finished = false;

        HttpRequestOptions::JsonCallback callback = [&](const QJsonObject &json) {
            finished = true;
            if (json.contains("error")) {
                errorOccurred = true;
                qDebug() << "Error occurred:" << json["error"].toInt();
            }
        };

        auto *reply = m_httpClient->sendRequest(
            HttpClient::Method::GET, errorCase.url, {}, {}, {.timeout = 3, .callback = callback});

        QTRY_VERIFY_WITH_TIMEOUT(finished, 5000);
        QCOMPARE(errorOccurred, errorCase.expectError);

        if (reply) {
            cleanupReply(reply);
        }
    }
}

void HttpClientTest::testCancelMechanism()
{
    // 测试立即取消
    {
        bool callbackCalled = false;

        HttpRequestOptions::JsonCallback callback
            = [&](const QJsonObject &) { callbackCalled = true; };

        auto *reply = m_httpClient->sendRequest(
            HttpClient::Method::GET, m_baseUrl + "/test", {}, {}, {.callback = callback});

        // 立即取消
        m_httpClient->cancel(reply);

        QTest::qWait(100);
        QVERIFY(!callbackCalled);
    }

    // 测试进行中取消
    {
        QEventLoop loop;
        bool callbackCalled = false;

        HttpRequestOptions::JsonCallback callback = [&](const QJsonObject &) {
            callbackCalled = true;
            loop.quit();
        };

        auto *reply = m_httpClient->sendRequest(
            HttpClient::Method::GET, m_baseUrl + "/test", {}, {}, {.callback = callback});

        QTimer::singleShot(50, [this, reply]() { m_httpClient->cancel(reply); });

        QTimer::singleShot(1000, &loop, &QEventLoop::quit);
        loop.exec();

        // 主要验证不会崩溃，回调是否被调用取决于取消时机
        QVERIFY(true);
    }
}

void HttpClientTest::testBoundaryConditions()
{
    // 测试空URL
    auto *reply1 = m_httpClient->sendRequest(HttpClient::Method::GET, QUrl(), {}, {});
    QJsonObject response1 = waitForReply(reply1);
    QVERIFY(response1.contains("error"));
    cleanupReply(reply1);

    // 测试自定义headers
    HttpClient::HttpHeaders headers;
    headers["X-Custom-Header"] = "CustomValue";
    headers["Authorization"] = "Bearer test-token";

    auto *reply2
        = m_httpClient->sendRequest(HttpClient::Method::GET, m_baseUrl + "/headers", headers, {});
    QJsonObject response2 = waitForReply(reply2);
    QVERIFY(!response2.isEmpty());
    QVERIFY(response2.contains("headers"));
    cleanupReply(reply2);

    // 测试特殊字符
    HttpClient::HttpHeaders specialHeaders{{"X-Special", "Header with spaces and \t tabs"},
                                           {"X-Unicode", "测试 🚀 emoji"}};

    auto *reply3 = m_httpClient->sendRequest(
        HttpClient::Method::GET, m_baseUrl + "/headers", specialHeaders, {});
    QJsonObject response3 = waitForReply(reply3);
    QVERIFY(!response3.isEmpty());
    cleanupReply(reply3);
}

void HttpClientTest::testDownloadEdgeCases()
{
    // 测试下载到已存在的文件
    QString existingPath = m_tempDir.filePath("existing_file.txt");
    QFile existingFile(existingPath);
    QVERIFY(existingFile.open(QIODevice::WriteOnly));
    existingFile.write("Existing content");
    existingFile.close();

    auto *reply1 = m_httpClient->downLoad(QUrl("http://127.0.0.1:8000/download"), existingPath);
    QJsonObject response1 = waitForReply(reply1);
    QVERIFY(QFile::exists(existingPath));
    cleanupReply(reply1);

    // 测试无效的下载路径
    QString invalidPath = "/invalid/path/test.txt";
    auto *reply2 = m_httpClient->downLoad(QUrl("http://127.0.0.1:8000/download"), invalidPath);

    // 对于无效路径，reply可能为nullptr或包含错误
    if (reply2) {
        QJsonObject response2 = waitForReply(reply2);
        // 响应可能包含错误信息
        QVERIFY(true); // 主要验证不会崩溃
        cleanupReply(reply2);
    }
}

// 功能特性测试
void HttpClientTest::testSyncRequest()
{
    auto *reply = createBasicGetRequest();
    QJsonObject response = m_httpClient->sync(reply);

    verifySuccessfulResponse(response);
    QCOMPARE(response["method"].toString(), QString("GET"));

    cleanupReply(reply);
}

void HttpClientTest::testCustomHeaders()
{
    HttpClient::HttpHeaders headers;
    headers["X-Custom-Header"] = "CustomValue";
    headers["Authorization"] = "Bearer test-token";
    headers["X-Test-Number"] = "12345";

    auto *reply
        = m_httpClient->sendRequest(HttpClient::Method::GET, m_baseUrl + "/headers", headers, {});

    QJsonObject response = waitForReply(reply);
    QVERIFY(!response.isEmpty());
    QVERIFY(response.contains("headers"));

    auto responseHeaders = response["headers"].toObject();
    QVERIFY(responseHeaders.contains("X-Custom-Header"));
    QCOMPARE(responseHeaders["X-Custom-Header"].toString(), QString("CustomValue"));

    cleanupReply(reply);
}

void HttpClientTest::testMultipleRequests()
{
    const int totalRequests = 3;
    QVector<QJsonObject> responses;
    QVector<QNetworkReply *> replies;
    QEventLoop loop;
    std::atomic<int> completedRequests{0};

    auto sharedCallback = LifecycleCallback<const QJsonObject &>([&](const QJsonObject &json) {
        responses.append(json);
        completedRequests++;

        if (completedRequests == totalRequests) {
            loop.quit();
        }
    });

    for (int i = 0; i < totalRequests; ++i) {
        auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                                m_baseUrl + "/test",
                                                {{"X-Request-ID", QString::number(i)}},
                                                {},
                                                {.timeout = 30, .callback = sharedCallback});
        QVERIFY(reply != nullptr);
        replies.append(reply);
    }

    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeoutTimer.start(30000);

    loop.exec();

    QCOMPARE(completedRequests, totalRequests);
    QCOMPARE(responses.size(), totalRequests);

    for (const auto &response : responses) {
        verifySuccessfulResponse(response);
    }

    for (auto *reply : replies) {
        cleanupReply(reply);
    }
}

void HttpClientTest::testConcurrentRequests()
{
    const int concurrentCount = 5;
    QVector<QNetworkReply *> replies;
    QVector<QJsonObject> responses;
    QEventLoop loop;
    std::atomic<int> completedCount{0};

    HttpRequestOptions::JsonCallback callback = [&](const QJsonObject &json) {
        responses.append(json);
        completedCount++;

        if (completedCount == concurrentCount) {
            loop.quit();
        }
    };

    for (int i = 0; i < concurrentCount; ++i) {
        auto *reply = m_httpClient->sendRequest(
            HttpClient::Method::GET, m_baseUrl + "/test", {}, {}, {.callback = callback});
        replies.append(reply);
    }

    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();

    QCOMPARE(completedCount, concurrentCount);
    QCOMPARE(responses.size(), concurrentCount);

    for (const auto &response : responses) {
        verifySuccessfulResponse(response);
    }

    for (auto *reply : replies) {
        cleanupReply(reply);
    }
}

void HttpClientTest::testHttpClientStateManagement()
{
    const int requestCount = 3;
    QVector<QNetworkReply *> replies;
    QVector<QJsonObject> responses;
    QMutex mutex;

    HttpRequestOptions::JsonCallback callback = [&](const QJsonObject &json) {
        QMutexLocker locker(&mutex);
        responses.append(json);
    };

    for (int i = 0; i < requestCount; ++i) {
        auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                                m_baseUrl + "/test",
                                                {{"X-Request-ID", QString::number(i)}},
                                                {},
                                                {.callback = callback});
        replies.append(reply);
    }

    QTRY_VERIFY_WITH_TIMEOUT(responses.size() == requestCount, 10000);

    for (const auto &response : responses) {
        verifySuccessfulResponse(response);
    }

    for (auto *reply : replies) {
        cleanupReply(reply);
    }

    // 验证HttpClient状态正常，可以继续使用
    auto *newReply = createBasicGetRequest();
    QJsonObject newResponse = waitForReply(newReply);
    verifySuccessfulResponse(newResponse);
    cleanupReply(newReply);
}

QTEST_MAIN(HttpClientTest)

#include "httpclient_unittest.moc"
