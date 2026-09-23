#include "httpclient.hpp"

#include <LifecycleCallback/lifecyclecallback_qt.hpp>

#include <QFile>
#include <QJsonArray>
#include <QJsonValue>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;

class CallbackReceiver : public QObject
{
public:
    int finishedCount = 0;
    int successCount = 0;
    int progressCount = 0;
    HttpResult lastResult;

    struct
    {
        qint64 received = -1;
        qint64 total = -1;
    } lastProgress;

    void handleFinished(const HttpResult &result)
    {
        ++finishedCount;
        lastResult = result;
        if (result.success()) {
            ++successCount;
        }
    }

    void handleProgress(qint64 received, qint64 total)
    {
        ++progressCount;
        lastProgress = {received, total};
    }
};

// Integration tests: require the Flask test server (unittests/test_server.py)
// running on http://127.0.0.1:8000. All tests are skipped when it is unreachable.
class HttpClientTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    // JSON requests
    void testSyncGet();
    void testSyncPostJson();
    void testSyncPutJson();
    void testSyncDelete();
    void testJsonValueRoundTrip_data();
    void testJsonValueRoundTrip();
    void testPatchViaGenericRequest();
    void testCustomHeaders();
    void testSpecialCharacterHeaders();
    void testBearerAuth();
    void testBasicAuth();
    void testQueryParameters();
    void testQueryUrlEncoding();
    void testEmptyJsonPostSendsNoBody();
    void testNonJsonResponse();
    void testResponseHeaderLookup();
    void testServerErrorStatus_data();
    void testServerErrorStatus();

    // Async behavior
    void testAsyncFinishedCallback();
    void testSyncTimeout();
    void testCancelRunningRequest();
    void testConcurrentRequests();
    void testProgressReachesTotal();

    // Download
    void testDownload();
    void testDownloadProgress();
    void testSyncDownloadProgress();
    void testDownloadResume();
    void testDownloadResumeProgress();
    void testDownloadPostBody();
    void testDownloadPostBodyResume();
    void testDownloadNoRangeRestartsFromScratch();
    void testDownloadOverwritesExistingFile();
    void testDownloadHttpErrorDiscardsTemp();
    void testDownloadRenameFailureProducesFileError();

    // SSL（自签 HTTPS 实例）
    void testSelfSignedSslFailsByDefault();
    void testIgnoreSslErrorsSucceeds();

    // Upload
    void testUploadPutFile();
    void testUploadPutData();
    void testUploadPostRawFile();
    void testUploadPostRawData();
    void testUploadMultipartFile();
    void testUploadMultipartData();
    void testUploadEmptyData();
    void testUploadProgress();
    void testLargeBodyRoundTrip();

private:
    [[nodiscard]] static bool isServerAvailable();

    HttpClient *m_client = nullptr;
    QString m_baseUrl;
    QTemporaryDir m_tempDir;
    bool m_serverAvailable = false;
    bool m_httpsAvailable = false;
};

void HttpClientTest::initTestCase()
{
    m_client = new HttpClient(this);
    m_baseUrl = u"http://127.0.0.1:8000"_s;
    QVERIFY(m_tempDir.isValid());
    m_serverAvailable = isServerAvailable();
    if (!m_serverAvailable) {
        qWarning() << "Test server not available at" << m_baseUrl << "- skipping integration tests";
        return;
    }
    // HTTPS 实例为可选能力（服务器缺 pyOpenSSL 时仅 HTTP）
    HttpClient probe;
    m_httpsAvailable = probe.get(QUrl(u"https://127.0.0.1:8443/api/health"_s))
                           .ignoreSslErrors()
                           .timeout(std::chrono::seconds(5))
                           .sync()
                           .success();
}

void HttpClientTest::cleanupTestCase()
{
    // 通知服务器清空上传目录，避免跨运行残留；尽力而为，不校验结果
    if (m_serverAvailable) {
        const auto cleanup = m_client->post(QUrl(m_baseUrl + u"/api/cleanup"_s))
                                 .timeout(std::chrono::seconds(5))
                                 .sync();
        Q_UNUSED(cleanup);
    }
    // m_client is owned by the Qt parent-child tree
}

void HttpClientTest::init()
{
    if (!m_serverAvailable) {
        QSKIP("Test server not available");
    }
}

bool HttpClientTest::isServerAvailable()
{
    HttpClient probe;
    const auto result = probe.get(QUrl(u"http://127.0.0.1:8000/api/health"_s))
                            .timeout(std::chrono::seconds(3))
                            .sync();
    return result.success();
}

void HttpClientTest::testSyncGet()
{
    const auto result
        = m_client->get(QUrl(m_baseUrl + u"/api/test"_s)).timeout(std::chrono::seconds(10)).sync();
    QVERIFY(result.success());
    QCOMPARE(result.status, 200);
    QVERIFY(!result.json.isNull());
    QCOMPARE(result.json.object().value(u"method"_s).toString(), u"GET"_s);
}

void HttpClientTest::testSyncPostJson()
{
    QJsonObject body;
    body[u"name"_s] = u"foo"_s;
    body[u"count"_s] = 42;

    const auto result = m_client->post(QUrl(m_baseUrl + u"/api/echo"_s), body)
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(result.success());
    const auto parsed = result.json.object().value(u"parsed_json"_s).toObject();
    QCOMPARE(parsed.value(u"name"_s).toString(), u"foo"_s);
    QCOMPARE(parsed.value(u"count"_s).toInt(), 42);
}

void HttpClientTest::testSyncPutJson()
{
    QJsonObject body;
    body[u"item"_s] = u"widget"_s;
    body[u"quantity"_s] = 7;

    const auto result = m_client->put(QUrl(m_baseUrl + u"/api/echo"_s), body)
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(result.success());
    const auto parsed = result.json.object().value(u"parsed_json"_s).toObject();
    QCOMPARE(parsed.value(u"item"_s).toString(), u"widget"_s);
    QCOMPARE(parsed.value(u"quantity"_s).toInt(), 7);
}

void HttpClientTest::testSyncDelete()
{
    const auto result
        = m_client->del(QUrl(m_baseUrl + u"/api/test"_s)).timeout(std::chrono::seconds(10)).sync();
    QVERIFY(result.success());
    QCOMPARE(result.status, 200);
    QCOMPARE(result.json.object().value(u"method"_s).toString(), u"DELETE"_s);
}

void HttpClientTest::testJsonValueRoundTrip_data()
{
    QTest::addColumn<QJsonValue>("value");

    QTest::newRow("string") << QJsonValue(u"value"_s);
    QTest::newRow("number") << QJsonValue(42);
    QTest::newRow("float") << QJsonValue(3.14);
    QTest::newRow("bool-true") << QJsonValue(true);
    QTest::newRow("bool-false") << QJsonValue(false);
    QTest::newRow("null") << QJsonValue(QJsonValue::Null);
    QTest::newRow("array") << QJsonValue(QJsonArray{1, 2, 3});
    QTest::newRow("nested-object") << QJsonValue(QJsonObject{{u"key"_s, u"value"_s}});
}

void HttpClientTest::testJsonValueRoundTrip()
{
    QFETCH(QJsonValue, value);

    QJsonObject body;
    body[u"field"_s] = value;

    const auto result = m_client->post(QUrl(m_baseUrl + u"/api/echo"_s), body)
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(result.success());
    const auto parsed = result.json.object().value(u"parsed_json"_s).toObject();
    QCOMPARE(parsed.value(u"field"_s), value);
}

void HttpClientTest::testPatchViaGenericRequest()
{
    const auto result
        = m_client->request(HttpClient::Method::Patch, QUrl(m_baseUrl + u"/api/test"_s))
              .timeout(std::chrono::seconds(10))
              .sync();
    QVERIFY(result.success());
    QCOMPARE(result.json.object().value(u"method"_s).toString(), u"PATCH"_s);
}

void HttpClientTest::testCustomHeaders()
{
    const auto result = m_client->get(QUrl(m_baseUrl + u"/api/headers"_s))
                            .header("X-Custom-Header", "CustomValue")
                            .header("Authorization", "Bearer test-token")
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(result.success());
    const auto headers = result.json.object().value(u"headers"_s).toObject();
    QCOMPARE(headers.value(u"X-Custom-Header"_s).toString(), u"CustomValue"_s);
    QCOMPARE(headers.value(u"Authorization"_s).toString(), u"Bearer test-token"_s);
}

void HttpClientTest::testSpecialCharacterHeaders()
{
    const auto unicodeValue = u"测试 emoji"_s;
    const auto result = m_client->get(QUrl(m_baseUrl + u"/api/headers"_s))
                            .header("X-Special", "Header with spaces and \t tabs")
                            .header("X-Unicode", unicodeValue.toUtf8())
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(result.success());
    const auto headers = result.json.object().value(u"headers"_s).toObject();
    QCOMPARE(headers.value(u"X-Special"_s).toString(), u"Header with spaces and \t tabs"_s);
    // Header 值按字节传输；服务端（Werkzeug）以 latin-1 解码，
    // 故 UTF-8 字节会呈现为 latin-1 视图，往返比对需按同一方式解码
    QCOMPARE(headers.value(u"X-Unicode"_s).toString(), QString::fromLatin1(unicodeValue.toUtf8()));
}

void HttpClientTest::testBearerAuth()
{
    const auto result = m_client->get(QUrl(m_baseUrl + u"/api/headers"_s))
                            .bearer("test-token-123")
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(result.success());
    const auto headers = result.json.object().value(u"headers"_s).toObject();
    QCOMPARE(headers.value(u"Authorization"_s).toString(), u"Bearer test-token-123"_s);
}

void HttpClientTest::testBasicAuth()
{
    const auto result = m_client->get(QUrl(m_baseUrl + u"/api/headers"_s))
                            .basicAuth(u"alice"_s, u"s3cret:_pass"_s)
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(result.success());
    const auto headers = result.json.object().value(u"headers"_s).toObject();
    // base64("alice:s3cret:_pass") 独立预计算，与实现解耦
    QCOMPARE(headers.value(u"Authorization"_s).toString(), u"Basic YWxpY2U6czNjcmV0Ol9wYXNz"_s);
}

void HttpClientTest::testQueryParameters()
{
    const auto result = m_client->get(QUrl(m_baseUrl + u"/api/test"_s))
                            .query(u"page"_s, u"2"_s)
                            .query(u"size"_s, u"50"_s)
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(result.success());
    const auto args = result.json.object().value(u"query_args"_s).toObject();
    QCOMPARE(args.value(u"page"_s).toString(), u"2"_s);
    QCOMPARE(args.value(u"size"_s).toString(), u"50"_s);
}

void HttpClientTest::testQueryUrlEncoding()
{
    const auto result = m_client->get(QUrl(m_baseUrl + u"/api/test"_s))
                            .query(u"q"_s, u"hello world & foo=bar"_s)
                            .query(u"lang"_s, u"你好"_s)
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(result.success());
    const auto args = result.json.object().value(u"query_args"_s).toObject();
    QCOMPARE(args.value(u"q"_s).toString(), u"hello world & foo=bar"_s);
    QCOMPARE(args.value(u"lang"_s).toString(), u"你好"_s);
}

void HttpClientTest::testEmptyJsonPostSendsNoBody()
{
    // 空 JSON 对象是默认参数：请求不带 body 和 Content-Type
    const auto result
        = m_client->post(QUrl(m_baseUrl + u"/api/echo"_s)).timeout(std::chrono::seconds(10)).sync();
    QVERIFY(result.success());
    QCOMPARE(result.json.object().value(u"content_length"_s).toInt(), 0);
}

void HttpClientTest::testNonJsonResponse()
{
    const auto result
        = m_client->get(QUrl(m_baseUrl + u"/api/text"_s)).timeout(std::chrono::seconds(10)).sync();
    QVERIFY(result.success());
    QCOMPARE(result.status, 200);
    QVERIFY(result.json.isNull()); // body is not JSON, json stays null
    QVERIFY(result.body.contains("plain text"));
}

void HttpClientTest::testResponseHeaderLookup()
{
    const auto result
        = m_client->get(QUrl(m_baseUrl + u"/api/text"_s)).timeout(std::chrono::seconds(10)).sync();
    QVERIFY(result.success());
    QVERIFY(result.header("content-TYPE").startsWith("text/plain")); // 大小写不敏感查找
    QVERIFY(result.header("X-Does-Not-Exist").isEmpty());
}

void HttpClientTest::testServerErrorStatus_data()
{
    QTest::addColumn<int>("status");
    QTest::newRow("bad-request") << 400;
    QTest::newRow("not-found") << 404;
    QTest::newRow("internal-error") << 500;
}

void HttpClientTest::testServerErrorStatus()
{
    QFETCH(int, status);
    const auto result = m_client->get(QUrl(m_baseUrl + u"/api/error?type=%1"_s.arg(status)))
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(!result.success());
    QCOMPARE(result.status, status);
    // Server error status maps to a QNetworkReply error code value (< 1000)
    QVERIFY(result.code != HttpErrorCode::NoError);
    QVERIFY(result.code != HttpErrorCode::Timeout);
    QVERIFY(result.code != HttpErrorCode::FileError);
    QVERIFY(static_cast<int>(result.code) < 1000);
    QVERIFY(!result.message.isEmpty());
    // Error body is readable and parseable
    QVERIFY(!result.body.isEmpty());
    QVERIFY(!result.json.isNull());
}

void HttpClientTest::testAsyncFinishedCallback()
{
    CallbackReceiver receiver;
    m_client->get(QUrl(m_baseUrl + u"/api/test"_s))
        .timeout(std::chrono::seconds(10))
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send();
    QTRY_COMPARE_WITH_TIMEOUT(receiver.finishedCount, 1, 15000);
    QVERIFY(receiver.lastResult.success());
    QCOMPARE(receiver.lastResult.json.object().value(u"method"_s).toString(), u"GET"_s);
}

void HttpClientTest::testSyncTimeout()
{
    const auto result = m_client->get(QUrl(m_baseUrl + u"/api/timeout?delay=5"_s))
                            .timeout(std::chrono::seconds(1))
                            .sync();
    QCOMPARE(result.code, HttpErrorCode::Timeout);
    QVERIFY(!result.success());
    QVERIFY(!result.message.isEmpty());
}

void HttpClientTest::testCancelRunningRequest()
{
    CallbackReceiver receiver;
    auto task = m_client->get(QUrl(m_baseUrl + u"/api/timeout?delay=5"_s))
                    .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
                    .send();
    QTest::qWait(200); // request is in flight now
    task.cancel();
    QTest::qWait(500);
    // Cancelled requests do not emit onFinished
    QCOMPARE(receiver.finishedCount, 0);
}

void HttpClientTest::testConcurrentRequests()
{
    const int total = 5;
    CallbackReceiver receiver;
    for (int i = 0; i < total; ++i) {
        m_client->get(QUrl(m_baseUrl + u"/api/concurrent"_s))
            .timeout(std::chrono::seconds(15))
            .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
            .send();
    }
    QTRY_COMPARE_WITH_TIMEOUT(receiver.finishedCount, total, 30000);
    QCOMPARE(receiver.successCount, total);
}

void HttpClientTest::testProgressReachesTotal()
{
    const auto path = m_tempDir.filePath(u"progress_total.txt"_s);

    CallbackReceiver receiver;
    m_client->download(QUrl(m_baseUrl + u"/download"_s), path)
        .timeout(std::chrono::seconds(15))
        .onProgress(makeLifecycleCallback(&receiver, &CallbackReceiver::handleProgress))
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send();
    QTRY_COMPARE_WITH_TIMEOUT(receiver.finishedCount, 1, 15000);
    QVERIFY(receiver.lastProgress.total > 0); // 服务器提供 Content-Length
    QCOMPARE(receiver.lastProgress.received, receiver.lastProgress.total); // 进度收敛于完整大小
}

void HttpClientTest::testDownload()
{
    const auto path = m_tempDir.filePath(u"download.txt"_s);
    const auto result = m_client->download(QUrl(m_baseUrl + u"/download"_s), path)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    QVERIFY(!QFile::exists(path + u".temp"_s)); // 成功后 .temp 已改名消失
    QCOMPARE(result.status, 200);

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const auto content = file.readAll();
    QVERIFY(content.contains("test file content"));
}

void HttpClientTest::testDownloadProgress()
{
    const auto path = m_tempDir.filePath(u"progress.bin"_s);
    CallbackReceiver receiver;
    m_client->download(QUrl(m_baseUrl + u"/download"_s), path)
        .onProgress(makeLifecycleCallback(&receiver, &CallbackReceiver::handleProgress))
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(path), 15000);
    QVERIFY(receiver.progressCount > 0);
}

void HttpClientTest::testSyncDownloadProgress()
{
    // sync() 阻塞期间 QEventLoop 正常派发信号：onProgress 必须照常回调；
    // onFinished 仍被忽略——结果经返回值直接交付，不走回调
    const auto path = m_tempDir.filePath(u"sync_progress.bin"_s);
    CallbackReceiver receiver;
    const auto result
        = m_client->download(QUrl(m_baseUrl + u"/download"_s), path)
              .timeout(std::chrono::seconds(15))
              .onProgress(makeLifecycleCallback(&receiver, &CallbackReceiver::handleProgress))
              .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
              .sync();
    QVERIFY(result.success());
    QVERIFY(receiver.progressCount > 0); // 进度回调在 sync 下正常派发
    QCOMPARE(receiver.lastProgress.received, receiver.lastProgress.total); // 终值收敛
    QCOMPARE(receiver.finishedCount, 0); // onFinished 被忽略，无双重交付
}

void HttpClientTest::testDownloadResume()
{
    // Full download first: gives us the expected content
    const auto full = m_tempDir.filePath(u"resume_full.txt"_s);
    QVERIFY(m_client->download(QUrl(m_baseUrl + u"/download"_s), full)
                .timeout(std::chrono::seconds(15))
                .sync()
                .success());
    QFile fullFile(full);
    QVERIFY(fullFile.open(QIODevice::ReadOnly));
    const auto expected = fullFile.readAll();
    QVERIFY(expected.size() > 100);

    // Pre-create the .temp file with the first half of the content:
    // the next download must resume via a Range request
    const auto path = m_tempDir.filePath(u"resume.txt"_s);
    {
        QFile tempFile(path + u".temp"_s);
        QVERIFY(tempFile.open(QIODevice::WriteOnly));
        tempFile.write(expected.left(expected.size() / 2));
    }

    const auto result = m_client->download(QUrl(m_baseUrl + u"/download"_s), path)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    QCOMPARE(result.status, 206); // server honored the Range request

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), expected);
}

void HttpClientTest::testDownloadResumeProgress()
{
    // 参考内容：先完整下载一次
    const auto full = m_tempDir.filePath(u"resume_prog_full.txt"_s);
    QVERIFY(m_client->download(QUrl(m_baseUrl + u"/download"_s), full)
                .timeout(std::chrono::seconds(15))
                .sync()
                .success());
    QFile fullFile(full);
    QVERIFY(fullFile.open(QIODevice::ReadOnly));
    const auto expected = fullFile.readAll();
    QVERIFY(expected.size() > 100);

    // 预置半份 .temp：续传进度必须把 downloadBase 计入 received/total
    const auto path = m_tempDir.filePath(u"resume_prog.txt"_s);
    {
        QFile tempFile(path + u".temp"_s);
        QVERIFY(tempFile.open(QIODevice::WriteOnly));
        tempFile.write(expected.left(expected.size() / 2));
    }

    CallbackReceiver receiver;
    m_client->download(QUrl(m_baseUrl + u"/download"_s), path)
        .timeout(std::chrono::seconds(15))
        .onProgress(makeLifecycleCallback(&receiver, &CallbackReceiver::handleProgress))
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send();
    QTRY_COMPARE_WITH_TIMEOUT(receiver.finishedCount, 1, 15000);
    QCOMPARE(receiver.lastProgress.total, expected.size());    // total 含续传基数
    QCOMPARE(receiver.lastProgress.received, expected.size()); // 终值收敛于完整大小
}

void HttpClientTest::testDownloadPostBody()
{
    // POST + JSON 动作体的流式下载（RPC 风格网关）：method()/body() 正交配置
    const auto path = m_tempDir.filePath(u"download_post.txt"_s);
    QJsonObject body;
    body.insert("action", "download");
    body.insert("items", QJsonArray{"/volume1/alice/movie.mp4"});
    const auto result = m_client->download(QUrl(m_baseUrl + u"/download-post"_s), path)
                            .method(HttpClient::Method::Post)
                            .body(body)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    QCOMPARE(result.status, 200);
    QVERIFY(!QFile::exists(path + u".temp"_s)); // 成功后 .temp 已改名消失

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const auto content = file.readAll();
    QVERIFY(content.contains("test file content"));
}

void HttpClientTest::testDownloadPostBodyResume()
{
    // POST + body 下载的断点续传：Range 头随指定动词一并携带，服务器回 206 增量续传
    const auto full = m_tempDir.filePath(u"post_resume_full.txt"_s);
    QJsonObject body;
    body.insert("action", "download");
    body.insert("items", QJsonArray{"/volume1/alice/movie.mp4"});
    QVERIFY(m_client->download(QUrl(m_baseUrl + u"/download-post"_s), full)
                .method(HttpClient::Method::Post)
                .body(body)
                .timeout(std::chrono::seconds(15))
                .sync()
                .success());
    QFile fullFile(full);
    QVERIFY(fullFile.open(QIODevice::ReadOnly));
    const auto expected = fullFile.readAll();
    QVERIFY(expected.size() > 100);

    // 预置半份 .temp：下一次下载必须以 Range 请求续传
    const auto path = m_tempDir.filePath(u"post_resume.txt"_s);
    {
        QFile tempFile(path + u".temp"_s);
        QVERIFY(tempFile.open(QIODevice::WriteOnly));
        tempFile.write(expected.left(expected.size() / 2));
    }

    const auto result = m_client->download(QUrl(m_baseUrl + u"/download-post"_s), path)
                            .method(HttpClient::Method::Post)
                            .body(body)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    QCOMPARE(result.status, 206); // 服务器遵循了 Range 请求

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), expected);
}

void HttpClientTest::testDownloadNoRangeRestartsFromScratch()
{
    const auto path = m_tempDir.filePath(u"no_range.txt"_s);
    // 预置被污染的 .temp：服务器忽略 Range 返回 200 时必须截断重写而非拼接
    {
        QFile tempFile(path + u".temp"_s);
        QVERIFY(tempFile.open(QIODevice::WriteOnly));
        tempFile.write("STALE-JUNK");
    }
    const auto result = m_client->download(QUrl(m_baseUrl + u"/download-no-range"_s), path)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    QCOMPARE(result.status, 200);
    QVERIFY(!QFile::exists(path + u".temp"_s));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray expectedContent
        = QByteArray("This is test file content for download verification. ").repeated(10);
    QCOMPARE(file.readAll(), expectedContent);
}

void HttpClientTest::testDownloadOverwritesExistingFile()
{
    const auto path = m_tempDir.filePath(u"overwrite.txt"_s);
    {
        QFile stale(path);
        QVERIFY(stale.open(QIODevice::WriteOnly));
        stale.write("stale content that must be replaced");
    }
    const auto result = m_client->download(QUrl(m_baseUrl + u"/download"_s), path)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const auto content = file.readAll();
    QVERIFY(content.contains("test file content"));
    QVERIFY(!content.contains("stale"));
}

void HttpClientTest::testDownloadHttpErrorDiscardsTemp()
{
    const auto path = m_tempDir.filePath(u"error_dl.txt"_s);
    const auto result = m_client->download(QUrl(m_baseUrl + u"/api/error?type=500"_s), path)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(!result.success());
    QCOMPARE(result.status, 500);
    QVERIFY(!QFile::exists(path));              // 最终文件绝不生成
    QVERIFY(!QFile::exists(path + u".temp"_s)); // HTTP 错误体污染的 .temp 必须丢弃
}

void HttpClientTest::testUploadPutFile()
{
    const auto path = m_tempDir.filePath(u"upload_put.bin"_s);
    const QByteArray data(2048, 'X');
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(data);
    }

    const auto result = m_client->upload(QUrl(m_baseUrl + u"/api/upload"_s))
                            .putFile(path)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    QCOMPARE(result.json.object().value(u"content_length"_s).toInt(), data.size());
}

void HttpClientTest::testUploadPutData()
{
    const QByteArray data(4096, 'Y');
    const auto result = m_client->upload(QUrl(m_baseUrl + u"/api/upload"_s))
                            .putData(data)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    QCOMPARE(result.json.object().value(u"content_length"_s).toInt(), data.size());
}

void HttpClientTest::testUploadPostRawFile()
{
    // 一步上传契约：POST + 原始文件体流式 + X-Dest/X-Upload-Key/X-Conflict 头
    //（method() 改写 Upload 原始体载荷的动词；默认 PUT 行为不变，见 testUploadPutFile）
    const auto path = m_tempDir.filePath(u"upload_post.bin"_s);
    const QByteArray data(2048, 'R');
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(data);
    }

    const auto result = m_client->upload(QUrl(m_baseUrl + u"/upload-raw"_s))
                            .method(HttpClient::Method::Post)
                            .putFile(path)
                            .header("X-Dest", "/volume1/alice/photo.jpg")
                            .header("X-Upload-Key", "6f1c-uuid")
                            .header("X-Conflict", "rename")
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    const auto obj = result.json.object();
    QCOMPARE(obj.value(u"method"_s).toString(), u"POST"_s);
    QCOMPARE(obj.value(u"dest"_s).toString(), u"/volume1/alice/photo.jpg"_s);
    QCOMPARE(obj.value(u"upload_key"_s).toString(), u"6f1c-uuid"_s);
    QCOMPARE(obj.value(u"conflict"_s).toString(), u"rename"_s);
    QCOMPARE(obj.value(u"size"_s).toInt(), data.size());
}

void HttpClientTest::testUploadPostRawData()
{
    // putData + method() 改写：内存缓冲以指定动词发送（如 /chunk/ 分块）
    const QByteArray data(512, 'W');
    const auto result = m_client->upload(QUrl(m_baseUrl + u"/upload-raw"_s))
                            .method(HttpClient::Method::Post)
                            .putData(data)
                            .header("X-Dest", "/volume1/alice/part0.bin")
                            .header("X-Upload-Key", "6f1c-uuid")
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    const auto obj = result.json.object();
    QCOMPARE(obj.value(u"method"_s).toString(), u"POST"_s);
    QCOMPARE(obj.value(u"size"_s).toInt(), data.size());
}

void HttpClientTest::testUploadMultipartFile()
{
    const auto path = m_tempDir.filePath(u"upload_multipart.bin"_s);
    const QByteArray data(1536, 'Z');
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(data);
    }

    const auto result = m_client->upload(QUrl(m_baseUrl + u"/api/upload"_s))
                            .multipartFile(path)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    QCOMPARE(result.json.object().value(u"filename"_s).toString(), u"upload_multipart.bin"_s);
    QCOMPARE(result.json.object().value(u"content_length"_s).toInt(), data.size());
}

void HttpClientTest::testUploadMultipartData()
{
    const QByteArray data(768, 'W');
    const auto result = m_client->upload(QUrl(m_baseUrl + u"/api/upload"_s))
                            .multipartData(u"report.txt"_s, data)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    QCOMPARE(result.json.object().value(u"filename"_s).toString(), u"report.txt"_s);
    QCOMPARE(result.json.object().value(u"content_length"_s).toInt(), data.size());
}

void HttpClientTest::testUploadEmptyData()
{
    const auto result = m_client->upload(QUrl(m_baseUrl + u"/api/upload"_s))
                            .putData(QByteArray())
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QVERIFY(result.success());
    QCOMPARE(result.json.object().value(u"content_length"_s).toInt(), 0);
}

void HttpClientTest::testUploadProgress()
{
    const QByteArray data(64 * 1024, 'P');
    CallbackReceiver receiver;
    m_client->upload(QUrl(m_baseUrl + u"/api/upload"_s))
        .putData(data)
        .timeout(std::chrono::seconds(15))
        .onProgress(makeLifecycleCallback(&receiver, &CallbackReceiver::handleProgress))
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send();
    QTRY_COMPARE_WITH_TIMEOUT(receiver.finishedCount, 1, 20000);
    QVERIFY(receiver.progressCount > 0);
}

void HttpClientTest::testLargeBodyRoundTrip()
{
    const QByteArray data(1024 * 1024, 'L');
    const auto result = m_client->upload(QUrl(m_baseUrl + u"/api/upload"_s))
                            .putData(data)
                            .timeout(std::chrono::seconds(30))
                            .sync();
    QVERIFY(result.success());
    QCOMPARE(result.json.object().value(u"content_length"_s).toInt(), data.size());
}

void HttpClientTest::testDownloadRenameFailureProducesFileError()
{
    // 目标路径被同名目录占用：改名必败，必须报告 FileError 而非静默成功
    const auto path = m_tempDir.filePath(u"rename_blocker"_s);
    QVERIFY(QDir(m_tempDir.path()).mkdir(u"rename_blocker"_s));

    const auto result = m_client->download(QUrl(m_baseUrl + u"/download"_s), path)
                            .timeout(std::chrono::seconds(15))
                            .sync();
    QCOMPARE(result.code, HttpErrorCode::FileError);
    QVERIFY(!result.success());
    QVERIFY(!result.message.isEmpty());
    // 完整数据保留在 .temp，可手工恢复
    QVERIFY(QFile::exists(path + u".temp"_s));
    QVERIFY(QFile(path + u".temp"_s).size() > 100);
}

void HttpClientTest::testSelfSignedSslFailsByDefault()
{
    if (!m_httpsAvailable) {
        QSKIP("HTTPS test instance not available");
    }
    const auto result = m_client->get(QUrl(u"https://127.0.0.1:8443/api/health"_s))
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(!result.success());
    QCOMPARE(result.status, 0);           // 握手即失败，未到达应用层
    QVERIFY(static_cast<int>(result.code) > 0 && static_cast<int>(result.code) < 1000);
    QVERIFY(!result.sslErrors.isEmpty()); // 失败详情已收集进结果
}

void HttpClientTest::testIgnoreSslErrorsSucceeds()
{
    if (!m_httpsAvailable) {
        QSKIP("HTTPS test instance not available");
    }
    const auto result = m_client->get(QUrl(u"https://127.0.0.1:8443/api/health"_s))
                            .ignoreSslErrors()
                            .timeout(std::chrono::seconds(10))
                            .sync();
    QVERIFY(result.success());
    QVERIFY(!result.sslErrors.isEmpty()); // 忽略的同时错误详情仍带入结果
    QCOMPARE(result.json.object().value(u"status"_s).toString(), u"healthy"_s);
}

QTEST_MAIN(HttpClientTest)
#include "httpclient_unittest.moc"
