#include "httpclient.hpp"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTest>
#include <QTimer>

using namespace Qt::StringLiterals;

// 无网络测试：不需要测试服务器，验证取消、超时、错误处理等逻辑
class HttpClientNoNetTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 取消机制：立即取消请求，不应崩溃
    void testCancelImmediately();

    // 超时机制：不可达主机 + 短超时，验证 timeout 信号和 error 字段
    void testTimeoutWithUnreachableHost();

    // 错误回调：不可达主机触发 errorOccurred，验证回调被调用
    void testErrorCallbackOnUnreachableHost();

    // resultHook：验证 resultHook 对结果的变换
    void testResultHook();

    // 取消后不再触发回调
    void testNoCallbackAfterCancel();

    // Q_ENUM 反射：Method 枚举的元对象系统能正确产生方法名字符串
    void testMethodEnumReflection();

    // 断点续传：已有 .temp 文件时 downLoad 应设置 Range 头
    void testDownloadResumeWithExistingTempFile();

private:
    HttpClient *m_httpClient = nullptr;
    QTemporaryDir m_tempDir;
};

void HttpClientNoNetTest::initTestCase()
{
    m_httpClient = new HttpClient(this);
    QVERIFY2(m_tempDir.isValid(), "Failed to create temporary directory");
}

void HttpClientNoNetTest::cleanupTestCase()
{
    // 测试对象由 Qt 父子关系自动释放
}

void HttpClientNoNetTest::testCancelImmediately()
{
    // 向不可达地址发请求，立即取消
    auto *reply
        = m_httpClient->sendRequest(HttpClient::Method::GET,
                                    QUrl("http://192.0.2.1/test"), // RFC 5737 TEST-NET，保证不可达
                                    {},
                                    {},
                                    {.timeout = 30});

    QVERIFY(reply != nullptr);
    m_httpClient->cancel(reply);

    // cancel 后 reply 已 deleteLater，等待事件循环处理
    QTest::qWait(50);
    // 不崩溃即通过
}

void HttpClientNoNetTest::testTimeoutWithUnreachableHost()
{
    // 1 秒超时访问不可达地址
    bool timeoutSignalReceived = false;
    QJsonObject callbackResult;

    connect(m_httpClient, &HttpClient::timeOut, this, [&]() { timeoutSignalReceived = true; });

    HttpRequestOptions::JsonCallback callback;
    callback = [&](const QJsonObject &json) { callbackResult = json; };

    auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                            QUrl("http://192.0.2.1/test"),
                                            {},
                                            {},
                                            {.timeout = 1, .callback = callback});

    // 等待超时触发
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    connect(m_httpClient, &HttpClient::ready, this, [&](QNetworkReply *r, const QJsonObject &) {
        if (r == reply) {
            loop.quit();
        }
    });
    loop.exec();

    QVERIFY2(timeoutSignalReceived, "timeout signal should be emitted");
    QVERIFY(callbackResult.contains("error"));
    QCOMPARE(callbackResult["error"].toInt(), HttpRequestOptions::NETWORK_TIMEOUT_ERROR);

    // 清理：queryResult 已断开信号并移除 TaskContext，但 reply 仍需手动释放
    reply->abort();
    reply->deleteLater();
}

void HttpClientNoNetTest::testErrorCallbackOnUnreachableHost()
{
    QJsonObject callbackResult;
    HttpRequestOptions::JsonCallback callback;
    callback = [&](const QJsonObject &json) { callbackResult = json; };

    auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                            QUrl("http://192.0.2.1/test"),
                                            {},
                                            {},
                                            {.timeout = 30, .callback = callback});

    // 等待错误完成
    QEventLoop loop;
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    connect(m_httpClient, &HttpClient::ready, this, [&](QNetworkReply *r, const QJsonObject &) {
        if (r == reply) {
            loop.quit();
        }
    });
    loop.exec();

    // 应收到包含 error 字段的回调
    QVERIFY2(callbackResult.contains("error"),
             "error callback should contain 'error' field for unreachable host");

    reply->deleteLater();
}

void HttpClientNoNetTest::testResultHook()
{
    // resultHook 将 {"value": 1} 变换为 {"value": 1, "hooked": true}
    QJsonObject callbackResult;
    HttpRequestOptions::JsonCallback callback;
    callback = [&](const QJsonObject &json) { callbackResult = json; };

    HttpRequestOptions::ResultHook hook = [](const QJsonObject &input) {
        auto result = input;
        result["hooked"] = true;
        return result;
    };

    auto *reply
        = m_httpClient->sendRequest(HttpClient::Method::GET,
                                    QUrl("http://192.0.2.1/test"),
                                    {},
                                    {},
                                    {.timeout = 1, .callback = callback, .resultHook = hook});

    // 等待完成（超时触发）
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    connect(m_httpClient, &HttpClient::ready, this, [&](QNetworkReply *r, const QJsonObject &) {
        if (r == reply) {
            loop.quit();
        }
    });
    loop.exec();

    // resultHook 应在 callback 之前执行，callback 收到的是 hook 处理后的结果
    QVERIFY2(callbackResult.contains("hooked"),
             "resultHook should add 'hooked' field before callback");
    QCOMPARE(callbackResult["hooked"].toBool(), true);

    reply->deleteLater();
}

void HttpClientNoNetTest::testNoCallbackAfterCancel()
{
    bool callbackFired = false;
    HttpRequestOptions::JsonCallback callback;
    callback = [&](const QJsonObject &) { callbackFired = true; };

    auto *reply = m_httpClient->sendRequest(HttpClient::Method::GET,
                                            QUrl("http://192.0.2.1/test"),
                                            {},
                                            {},
                                            {.timeout = 30, .callback = callback});

    // 立即取消
    m_httpClient->cancel(reply);

    // 等待一段时间，确认回调不会触发
    QTest::qWait(500);

    QVERIFY2(!callbackFired, "callback should not fire after cancel");
}

void HttpClientNoNetTest::testMethodEnumReflection()
{
    // 验证 Q_ENUM(Method) 的元对象反射能正确产生 HTTP 方法名字符串
    const auto metaEnum = QMetaEnum::fromType<HttpClient::Method>();

    QCOMPARE(metaEnum.valueToKey(static_cast<int>(HttpClient::Method::GET)), "GET");
    QCOMPARE(metaEnum.valueToKey(static_cast<int>(HttpClient::Method::POST)), "POST");
    QCOMPARE(metaEnum.valueToKey(static_cast<int>(HttpClient::Method::PUT)), "PUT");
    QCOMPARE(metaEnum.valueToKey(static_cast<int>(HttpClient::Method::DELETE)), "DELETE");

    // 反向查找：字符串 → 枚举值
    bool ok = false;
    QCOMPARE(metaEnum.keyToValue("GET", &ok), static_cast<int>(HttpClient::Method::GET));
    QVERIFY(ok);
    QCOMPARE(metaEnum.keyToValue("DELETE", &ok), static_cast<int>(HttpClient::Method::DELETE));
    QVERIFY(ok);

    // 枚举值数量
    QCOMPARE(metaEnum.keyCount(), 4);
}

void HttpClientNoNetTest::testDownloadResumeWithExistingTempFile()
{
    // 创建一个已有内容的 .temp 文件，模拟断点续传场景
    const auto filePath = m_tempDir.filePath(u"resume_test.txt"_s);
    const auto tempPath = filePath + u".temp"_s;

    {
        QFile tempFile(tempPath);
        QVERIFY2(tempFile.open(QIODevice::WriteOnly), "Failed to create temp file");
        tempFile.write("已有的部分数据");
        tempFile.close();
    }

    // downLoad 应检测到 .temp 已有数据，设置 Range 头（通过请求发出不崩溃验证）
    auto *reply
        = m_httpClient->downLoad(QUrl("http://192.0.2.1/download"), filePath, {.timeout = 1});

    QVERIFY2(reply != nullptr,
             "downLoad should return non-null reply for valid path with temp file");

    // 验证 Range 头已设置（.temp 文件 size > 0）
    const auto hasRangeHeader = reply->request().hasRawHeader("Range");
    QVERIFY2(hasRangeHeader, "Range header should be set when .temp file has existing content");

    // 取消请求，不需要等待完成
    m_httpClient->cancel(reply);
    QTest::qWait(50);

    // 清理临时文件
    QFile::remove(tempPath);
    QFile::remove(filePath);
}

QTEST_MAIN(HttpClientNoNetTest)
#include "httpclient_nonet_unittest.moc"
