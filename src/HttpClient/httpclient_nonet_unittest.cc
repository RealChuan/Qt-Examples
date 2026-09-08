#include "httpclient.hpp"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;

// 无网络测试：不需要测试服务器。
// 不可达地址统一使用 RFC 5737 TEST-NET-1 (192.0.2.1) 与 RFC 6761 .invalid TLD。
class HttpClientNoNetTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 取消语义
    void cancelImmediatelyDoesNotEmitFinished();
    void cancelWhileRunningDoesNotEmitFinished();
    void cancelIsIdempotent();

    // 超时与错误
    void timeoutProducesTimeoutError();
    void syncTimeoutReturnsTimeoutResult();
    void unreachableHostProducesNetworkError();
    void emptyUrlProducesError();

    // 句柄语义
    void droppedTaskStillFinishes();
    void taskCopySharesCancellation();
    void syncIgnoresUserCallbacks();
    void downloadTimeoutKeepsTempFile();

    // 文件错误
    void downloadWithInvalidPathProducesFileError();
    void uploadWithMissingFileProducesFileError();
    void uploadWithoutPayloadProducesFileError();

    // 上传中断
    void uploadTimeoutCancelsTransfer();

    // 构建器一次性
    void builderIsSingleUse();

    // Q_ENUM 反射
    void methodEnumReflection();

private:
    HttpClient *m_client = nullptr;
    QTemporaryDir m_tempDir;
};

void HttpClientNoNetTest::initTestCase()
{
    m_client = new HttpClient(this);
    QVERIFY2(m_tempDir.isValid(), "Failed to create temporary directory");
}

void HttpClientNoNetTest::cleanupTestCase()
{
    // m_client 由 Qt 父子关系自动释放
}

void HttpClientNoNetTest::cancelImmediatelyDoesNotEmitFinished()
{
    int finishedCount = 0;
    auto task = m_client->get(QUrl(u"http://192.0.2.1/test"_s))
                    .timeout(std::chrono::seconds(30))
                    .onFinished([&finishedCount](const HttpResult &) { ++finishedCount; })
                    .send();
    task.cancel();
    QTest::qWait(200);
    QCOMPARE(finishedCount, 0);
}

void HttpClientNoNetTest::cancelWhileRunningDoesNotEmitFinished()
{
    int finishedCount = 0;
    auto task = m_client->get(QUrl(u"http://192.0.2.1/test"_s))
                    .timeout(std::chrono::seconds(30))
                    .onFinished([&finishedCount](const HttpResult &) { ++finishedCount; })
                    .send();
    QTest::qWait(50); // 确保请求已在途
    task.cancel();
    QTest::qWait(200);
    QCOMPARE(finishedCount, 0);
}

void HttpClientNoNetTest::cancelIsIdempotent()
{
    auto task
        = m_client->get(QUrl(u"http://192.0.2.1/test"_s)).timeout(std::chrono::seconds(30)).send();
    task.cancel();
    task.cancel(); // 第二次取消为安全空操作
    QTest::qWait(100);
}

void HttpClientNoNetTest::timeoutProducesTimeoutError()
{
    HttpResult result;
    int finishedCount = 0;
    m_client->get(QUrl(u"http://192.0.2.1/test"_s))
        .timeout(std::chrono::seconds(1))
        .onFinished([&result, &finishedCount](const HttpResult &r) {
            result = r;
            ++finishedCount;
        })
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(finishedCount == 1, 10000);
    QCOMPARE(result.code, HttpErrorCode::Timeout);
    QVERIFY(!result.success());
    QVERIFY(!result.message.isEmpty());
    QCOMPARE(result.status, 0);
}

void HttpClientNoNetTest::syncTimeoutReturnsTimeoutResult()
{
    const auto result
        = m_client->get(QUrl(u"http://192.0.2.1/test"_s)).timeout(std::chrono::seconds(1)).sync();
    QCOMPARE(result.code, HttpErrorCode::Timeout);
    QVERIFY(!result.success());
    QVERIFY(!result.message.isEmpty());
}

void HttpClientNoNetTest::unreachableHostProducesNetworkError()
{
    HttpResult result;
    int finishedCount = 0;
    // .invalid TLD 保证 DNS 解析立即失败，无需等待 TCP 超时
    m_client->get(QUrl(u"http://nonexistent-host-xyz-12345.invalid"_s))
        .timeout(std::chrono::seconds(30))
        .onFinished([&result, &finishedCount](const HttpResult &r) {
            result = r;
            ++finishedCount;
        })
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(finishedCount == 1, 30000);
    QVERIFY(!result.success());
    QVERIFY(result.code != HttpErrorCode::NoError);
    QVERIFY(result.code != HttpErrorCode::Timeout);
    QVERIFY(result.code != HttpErrorCode::FileError);
    QVERIFY(!result.message.isEmpty());
}

void HttpClientNoNetTest::emptyUrlProducesError()
{
    HttpResult result;
    int finishedCount = 0;
    m_client->get(QUrl())
        .onFinished([&result, &finishedCount](const HttpResult &r) {
            result = r;
            ++finishedCount;
        })
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(finishedCount == 1, 10000);
    QVERIFY(!result.success());
    QVERIFY(result.code != HttpErrorCode::NoError);
    QVERIFY(!result.message.isEmpty());
}

void HttpClientNoNetTest::droppedTaskStillFinishes()
{
    int finishedCount = 0;
    m_client->get(QUrl(u"http://192.0.2.1/test"_s))
        .timeout(std::chrono::seconds(1))
        .onFinished([&finishedCount](const HttpResult &) { ++finishedCount; })
        .send(); // 句柄直接丢弃：fire-and-forget，请求照常完成
    QTRY_VERIFY_WITH_TIMEOUT(finishedCount == 1, 10000);
}

void HttpClientNoNetTest::downloadWithInvalidPathProducesFileError()
{
    // 路径穿过一个普通文件（blocker.txt/sub.txt），保证打开必败且跨平台
    const auto blocker = m_tempDir.filePath(u"blocker.txt"_s);
    {
        QFile file(blocker);
        QVERIFY(file.open(QIODevice::WriteOnly));
    }

    int finishedCount = 0;
    HttpResult result;
    m_client->download(QUrl(u"http://192.0.2.1/file"_s), blocker + u"/sub.txt"_s)
        .onFinished([&result, &finishedCount](const HttpResult &r) {
            result = r;
            ++finishedCount;
        })
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(finishedCount == 1, 5000);
    QCOMPARE(result.code, HttpErrorCode::FileError);
    QVERIFY(!result.message.isEmpty());

    // sync 路径同样立即返回 FileError
    const auto syncResult
        = m_client->download(QUrl(u"http://192.0.2.1/file"_s), blocker + u"/sub2.txt"_s).sync();
    QCOMPARE(syncResult.code, HttpErrorCode::FileError);
}

void HttpClientNoNetTest::uploadWithMissingFileProducesFileError()
{
    const auto missing = m_tempDir.filePath(u"missing.bin"_s);
    const auto result
        = m_client->upload(QUrl(u"http://192.0.2.1/upload"_s)).putFile(missing).sync();
    QCOMPARE(result.code, HttpErrorCode::FileError);
    QVERIFY(!result.message.isEmpty());

    int finishedCount = 0;
    m_client->upload(QUrl(u"http://192.0.2.1/upload"_s))
        .multipartFile(missing)
        .onFinished([&finishedCount](const HttpResult &) { ++finishedCount; })
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(finishedCount == 1, 5000);
}

void HttpClientNoNetTest::uploadWithoutPayloadProducesFileError()
{
    // 未配置载荷：统一返回 FileError；回调延迟到事件循环派发（与 send() 的异步契约一致）
    HttpResult result;
    int finishedCount = 0;
    m_client->upload(QUrl(u"http://192.0.2.1/upload"_s))
        .onFinished([&result, &finishedCount](const HttpResult &r) {
            result = r;
            ++finishedCount;
        })
        .send();
    // 错误回调延迟到事件循环派发：send() 返回时尚未执行
    QCOMPARE(finishedCount, 0);
    QTRY_VERIFY_WITH_TIMEOUT(finishedCount == 1, 5000);
    QCOMPARE(result.code, HttpErrorCode::FileError);
    QVERIFY(!result.message.isEmpty());

    const auto syncResult = m_client->upload(QUrl(u"http://192.0.2.1/upload"_s)).sync();
    QCOMPARE(syncResult.code, HttpErrorCode::FileError);
}

void HttpClientNoNetTest::uploadTimeoutCancelsTransfer()
{
    // 上传源文件已打开时超时中断：覆盖 abort 时 TaskState 的清理路径
    const auto path = m_tempDir.filePath(u"upload_timeout.bin"_s);
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(256 * 1024, 'x'));
    }
    const auto result = m_client->upload(QUrl(u"http://192.0.2.1/upload"_s))
                            .putFile(path)
                            .timeout(std::chrono::seconds(1))
                            .sync();
    QCOMPARE(result.code, HttpErrorCode::Timeout);
    QVERIFY(!result.success());
}

void HttpClientNoNetTest::methodEnumReflection()
{
    const auto metaEnum = QMetaEnum::fromType<HttpClient::Method>();

    QCOMPARE(metaEnum.keyCount(), 5);
    QCOMPARE(metaEnum.valueToKey(static_cast<int>(HttpClient::Method::Get)), "Get");
    QCOMPARE(metaEnum.valueToKey(static_cast<int>(HttpClient::Method::Post)), "Post");
    QCOMPARE(metaEnum.valueToKey(static_cast<int>(HttpClient::Method::Put)), "Put");
    QCOMPARE(metaEnum.valueToKey(static_cast<int>(HttpClient::Method::Delete)), "Delete");
    QCOMPARE(metaEnum.valueToKey(static_cast<int>(HttpClient::Method::Patch)), "Patch");

    bool ok = false;
    QCOMPARE(metaEnum.keyToValue("Patch", &ok), static_cast<int>(HttpClient::Method::Patch));
    QVERIFY(ok);
}

void HttpClientNoNetTest::taskCopySharesCancellation()
{
    int finishedCount = 0;
    const auto task = m_client->get(QUrl(u"http://192.0.2.1/test"_s))
                          .timeout(std::chrono::seconds(30))
                          .onFinished([&finishedCount](const HttpResult &) { ++finishedCount; })
                          .send();
    const auto copy = task; // 拷贝与原句柄指向同一在途请求
    copy.cancel();
    QTest::qWait(200);
    QCOMPARE(finishedCount, 0);
}

void HttpClientNoNetTest::syncIgnoresUserCallbacks()
{
    bool finishedCalled = false;
    bool progressCalled = false;
    const auto result
        = m_client->get(QUrl(u"http://192.0.2.1/test"_s))
              .timeout(std::chrono::seconds(1))
              .onFinished([&finishedCalled](const HttpResult &) { finishedCalled = true; })
              .onProgress([&progressCalled](qint64, qint64) { progressCalled = true; })
              .sync();
    QCOMPARE(result.code, HttpErrorCode::Timeout);
    QVERIFY(!finishedCalled); // sync 的契约：忽略用户回调，只返回结果
    QVERIFY(!progressCalled);
}

void HttpClientNoNetTest::downloadTimeoutKeepsTempFile()
{
    const auto path = m_tempDir.filePath(u"timeout_dl.txt"_s);
    const auto result = m_client->download(QUrl(u"http://192.0.2.1/file"_s), path)
                            .timeout(std::chrono::seconds(1))
                            .sync();
    QCOMPARE(result.code, HttpErrorCode::Timeout);
    QCOMPARE(result.status, 0);
    QVERIFY(!QFile::exists(path));
    // 网络层失败（status==0）保留 .temp 供断点续传
    QVERIFY(QFile::exists(path + u".temp"_s));
}

void HttpClientNoNetTest::builderIsSingleUse()
{
    // 入口函数返回纯右值（移动构造）；链式配置返回左值引用，
    // 故先具名接收入口产物，再在其上链式配置，避免触发已删除的拷贝构造
    int finishedCount = 0;
    auto builder = m_client->get(QUrl(u"http://192.0.2.1/test"_s));
    builder.timeout(std::chrono::seconds(1)).onFinished([&finishedCount](const HttpResult &) {
        ++finishedCount;
    });
    builder.send();
    builder.send();                                      // 已消费：告警并忽略，不发起第二个请求
    QTRY_VERIFY_WITH_TIMEOUT(finishedCount == 1, 10000); // 首个请求恰好一次回调
    QTest::qWait(1500);                                  // 超出超时窗口后仍无第二次回调
    QCOMPARE(finishedCount, 1);

    auto syncBuilder = m_client->get(QUrl(u"http://192.0.2.1/test"_s));
    syncBuilder.timeout(std::chrono::seconds(1));
    (void) syncBuilder.sync(); // 首次：正常等待超时
    const auto reused = syncBuilder.sync();
    QCOMPARE(reused.code, HttpErrorCode::InvalidUse);
    QVERIFY(!reused.success());
    QVERIFY(!reused.message.isEmpty());
}

QTEST_MAIN(HttpClientNoNetTest)
#include "httpclient_nonet_unittest.moc"
