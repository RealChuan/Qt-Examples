#include "httpclient.hpp"

#include <LifecycleCallback/lifecyclecallback_qt.hpp>

#include <QFile>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTest>

#include <mutex>

using namespace Qt::StringLiterals;

// 无网络测试：不需要测试服务器。
// 不可达地址统一使用 RFC 5737 TEST-NET-1 (192.0.2.1) 与 RFC 6761 .invalid TLD。

namespace {

// 日志脱敏测试的捕获器：测试窗口内接管全部日志输出。
// message handler 可能被 QNAM 工作线程调用，互斥保护共享缓冲
std::mutex logMutex;
QString logBuffer;

void captureLog(QtMsgType, const QMessageLogContext &, const QString &msg)
{
    const std::lock_guard lock(logMutex);
    logBuffer.append(msg);
    logBuffer.append(u'\n');
}

} // namespace

class CallbackReceiver : public QObject
{
public:
    int finishedCount = 0;
    int progressCount = 0;
    HttpResult lastResult;

    void handleFinished(const HttpResult &result)
    {
        ++finishedCount;
        lastResult = result;
    }

    void handleProgress(qint64, qint64) { ++progressCount; }
};

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
    void cancelAfterFinishIsNoOp();

    // 超时与错误
    void timeoutProducesTimeoutError();
    void syncTimeoutReturnsTimeoutResult();
    void unreachableHostProducesNetworkError();
    void emptyUrlProducesError();

    // 句柄语义
    void droppedTaskStillFinishes();
    void taskCopySharesCancellation();
    void syncIgnoresFinishedCallback();
    void downloadTimeoutKeepsTempFile();

    // 响应侧扩展点与结果字段
    void resultReadyRunsBeforeFinishedCallback();
    void localFailureResultCarriesMethod();

    // 文件错误
    void downloadWithInvalidPathProducesFileError();
    void uploadWithMissingFileProducesFileError();
    void uploadWithoutPayloadProducesInvalidUse();
    void uploadBodyConflictProducesInvalidUse();
    void multipartVerbOverrideWarnsAndStaysPost();

    // 上传中断
    void uploadTimeoutCancelsTransfer();

    // 构建器一次性
    void builderIsSingleUse();
    void builderMovedFromIsSafeNoOp();

    // 日志脱敏
    void urlCredentialsRedactedInLogs();
    void urlQueryCredentialsRedactedInLogs();
    void sensitiveRequestHeadersRedactedInLogs();

    // mTLS 输入校验
    void clientCertNullInputWarnsAndIgnores();

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

void HttpClientNoNetTest::cleanupTestCase() {}

void HttpClientNoNetTest::cancelImmediatelyDoesNotEmitFinished()
{
    CallbackReceiver receiver;
    auto task = m_client->get(QUrl(u"http://192.0.2.1/test"_s))
                    .timeout(std::chrono::seconds(30))
                    .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
                    .send();
    task.cancel();
    QTest::qWait(200);
    QCOMPARE(receiver.finishedCount, 0);
}

void HttpClientNoNetTest::cancelWhileRunningDoesNotEmitFinished()
{
    CallbackReceiver receiver;
    auto task = m_client->get(QUrl(u"http://192.0.2.1/test"_s))
                    .timeout(std::chrono::seconds(30))
                    .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
                    .send();
    QTest::qWait(50); // 确保请求已在途
    task.cancel();
    QTest::qWait(200);
    QCOMPARE(receiver.finishedCount, 0);
}

void HttpClientNoNetTest::cancelIsIdempotent()
{
    auto task
        = m_client->get(QUrl(u"http://192.0.2.1/test"_s)).timeout(std::chrono::seconds(30)).send();
    task.cancel();
    task.cancel();
    QTest::qWait(100);
}

void HttpClientNoNetTest::cancelAfterFinishIsNoOp()
{
    // 任务终止（此处为超时）后 cancel：map 未命中分支的安全空操作，
    // 不得二次派发回调或干扰后续请求
    CallbackReceiver receiver;
    auto task = m_client->get(QUrl(u"http://192.0.2.1/test"_s))
                    .timeout(std::chrono::seconds(1))
                    .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
                    .send();
    QTRY_VERIFY_WITH_TIMEOUT(receiver.finishedCount == 1, 10000);
    task.cancel();
    QTest::qWait(100);
    QCOMPARE(receiver.finishedCount, 1);
}

void HttpClientNoNetTest::timeoutProducesTimeoutError()
{
    CallbackReceiver receiver;
    m_client->get(QUrl(u"http://192.0.2.1/test"_s))
        .timeout(std::chrono::seconds(1))
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(receiver.finishedCount == 1, 10000);
    QCOMPARE(receiver.lastResult.code, HttpErrorCode::Timeout);
    QVERIFY(!receiver.lastResult.success());
    QVERIFY(!receiver.lastResult.message.isEmpty());
    QCOMPARE(receiver.lastResult.status, 0);
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
    CallbackReceiver receiver;
    // .invalid TLD 保证 DNS 解析立即失败，无需等待 TCP 超时
    m_client->get(QUrl(u"http://nonexistent-host-xyz-12345.invalid"_s))
        .timeout(std::chrono::seconds(30))
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(receiver.finishedCount == 1, 30000);
    QVERIFY(!receiver.lastResult.success());
    QVERIFY(receiver.lastResult.code != HttpErrorCode::NoError);
    QVERIFY(receiver.lastResult.code != HttpErrorCode::Timeout);
    QVERIFY(receiver.lastResult.code != HttpErrorCode::FileError);
    QVERIFY(!receiver.lastResult.message.isEmpty());
}

void HttpClientNoNetTest::emptyUrlProducesError()
{
    CallbackReceiver receiver;
    m_client->get(QUrl())
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(receiver.finishedCount == 1, 10000);
    QVERIFY(!receiver.lastResult.success());
    QVERIFY(receiver.lastResult.code != HttpErrorCode::NoError);
    QVERIFY(!receiver.lastResult.message.isEmpty());
}

void HttpClientNoNetTest::droppedTaskStillFinishes()
{
    CallbackReceiver receiver;
    m_client->get(QUrl(u"http://192.0.2.1/test"_s))
        .timeout(std::chrono::seconds(1))
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send(); // 句柄直接丢弃：fire-and-forget，请求照常完成
    QTRY_VERIFY_WITH_TIMEOUT(receiver.finishedCount == 1, 10000);
}

void HttpClientNoNetTest::downloadWithInvalidPathProducesFileError()
{
    // 路径穿过一个普通文件（blocker.txt/sub.txt），保证打开必败且跨平台
    const auto blocker = m_tempDir.filePath(u"blocker.txt"_s);
    {
        QFile file(blocker);
        QVERIFY(file.open(QIODevice::WriteOnly));
    }

    CallbackReceiver receiver;
    m_client->download(QUrl(u"http://192.0.2.1/file"_s), blocker + u"/sub.txt"_s)
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(receiver.finishedCount == 1, 5000);
    QCOMPARE(receiver.lastResult.code, HttpErrorCode::FileError);
    QVERIFY(!receiver.lastResult.message.isEmpty());

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

    CallbackReceiver receiver;
    m_client->upload(QUrl(u"http://192.0.2.1/upload"_s))
        .multipartFile(missing)
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(receiver.finishedCount == 1, 5000);
}

void HttpClientNoNetTest::uploadWithoutPayloadProducesInvalidUse()
{
    // 未配置载荷属 API 误用：统一返回 InvalidUse；回调延迟到事件循环派发（与 send() 的异步契约一致）
    CallbackReceiver receiver;
    m_client->upload(QUrl(u"http://192.0.2.1/upload"_s))
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send();
    // 错误回调延迟到事件循环派发：send() 返回时尚未执行
    QCOMPARE(receiver.finishedCount, 0);
    QTRY_VERIFY_WITH_TIMEOUT(receiver.finishedCount == 1, 5000);
    QCOMPARE(receiver.lastResult.code, HttpErrorCode::InvalidUse);
    QVERIFY(!receiver.lastResult.message.isEmpty());

    const auto syncResult = m_client->upload(QUrl(u"http://192.0.2.1/upload"_s)).sync();
    QCOMPARE(syncResult.code, HttpErrorCode::InvalidUse);
}

void HttpClientNoNetTest::uploadBodyConflictProducesInvalidUse()
{
    // 载荷由 putFile/putData/multipart* 独占：body()/json() 与载荷冲突报 InvalidUse，
    // 而非静默忽略。冲突在 prepare 时判定（任何网络动作发生之前），无需真实网络
    const auto byBody = m_client->upload(QUrl(u"http://192.0.2.1/upload"_s))
                            .putData(QByteArray(64, 'x'))
                            .body("conflict", "text/plain")
                            .sync();
    QCOMPARE(byBody.code, HttpErrorCode::InvalidUse);
    QVERIFY(!byBody.message.isEmpty());

    const auto byJson = m_client->upload(QUrl(u"http://192.0.2.1/upload"_s))
                            .putData(QByteArray(64, 'x'))
                            .json(QJsonObject{})
                            .sync();
    QCOMPARE(byJson.code, HttpErrorCode::InvalidUse);
    QVERIFY(!byJson.message.isEmpty());
}

void HttpClientNoNetTest::multipartVerbOverrideWarnsAndStaysPost()
{
    // multipart 恒为 POST（QNAM 无自定义动词的 multipart 重载）：改写动词仅告警。
    // 告警在 prepare 期、文件打开前触发，配合缺失文件可免网络等待
    {
        const std::lock_guard lock(logMutex);
        logBuffer.clear();
    }
    const auto oldHandler = qInstallMessageHandler(captureLog);
    const auto result = m_client->upload(QUrl(u"http://192.0.2.1/upload"_s))
                            .multipartFile(m_tempDir.filePath(u"missing.bin"_s))
                            .method(HttpClient::Method::Put)
                            .sync();
    qInstallMessageHandler(oldHandler);

    QString captured;
    {
        const std::lock_guard lock(logMutex);
        captured = logBuffer;
    }
    QVERIFY(captured.contains(u"multipart upload is always POST"_s));
    QCOMPARE(result.code, HttpErrorCode::FileError); // 缺失文件错误：告警后仍走 multipart 路径
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

void HttpClientNoNetTest::urlCredentialsRedactedInLogs()
{
    // RFC 3986 userinfo 凭证：dispatched/timed out/finished 三条日志都会输出 URL，
    // 密码必须替换为占位符（super-secret 不得出现在任何日志行中），用户名保留以便排查
    {
        const std::lock_guard lock(logMutex);
        logBuffer.clear();
    }
    const auto oldHandler = qInstallMessageHandler(captureLog);
    const auto result = m_client->get(QUrl(u"http://user:super-secret@192.0.2.1/test"_s))
                            .timeout(std::chrono::seconds(1))
                            .sync();
    qInstallMessageHandler(oldHandler); // 恢复 Qt Test 输出，避免吞掉后续断言消息

    QString captured;
    {
        const std::lock_guard lock(logMutex);
        captured = logBuffer;
        logBuffer.clear();
    }
    QCOMPARE(result.code, HttpErrorCode::Timeout);
    QVERIFY2(!captured.contains(u"super-secret"_s), "URL password leaked into log output");
    QVERIFY(captured.contains(u"user:***@192.0.2.1"_s));
}

void HttpClientNoNetTest::urlQueryCredentialsRedactedInLogs()
{
    // query 凭证（预签名 URL / OAuth token）：敏感值必须替换为占位符，
    // 非敏感参数原样保留。键名子串匹配需覆盖现实变体：
    // access_token / X-Amz-Signature 等（大小写不敏感）
    {
        const std::lock_guard lock(logMutex);
        logBuffer.clear();
    }
    const auto oldHandler = qInstallMessageHandler(captureLog);
    const auto result
        = m_client
              ->get(QUrl(u"http://192.0.2.1/file?access_token=super-secret-token"
                         u"&X-Amz-Signature=abcd1234&X-Amz-Date=20260101T000000Z&page=2"_s))
              .timeout(std::chrono::seconds(1))
              .sync();
    qInstallMessageHandler(oldHandler);

    QString captured;
    {
        const std::lock_guard lock(logMutex);
        captured = logBuffer;
        logBuffer.clear();
    }
    QCOMPARE(result.code, HttpErrorCode::Timeout);
    QVERIFY2(!captured.contains(u"super-secret-token"_s), "query token leaked into log output");
    QVERIFY2(!captured.contains(u"abcd1234"_s), "presigned signature leaked into log output");
    QVERIFY(captured.contains(u"access_token=***"_s));
    QVERIFY(captured.contains(u"X-Amz-Signature=***"_s));
    QVERIFY(captured.contains(u"page=2"_s));                      // 非敏感参数原样保留
    QVERIFY(captured.contains(u"X-Amz-Date=20260101T000000Z"_s)); // 非凭证参数不误伤
}

void HttpClientNoNetTest::clientCertNullInputWarnsAndIgnores()
{
    // clientCert 空链/含空证书/空私钥属 API 误用：告警并忽略，请求正常发起
    //（超时终止即证明未被空配置破坏）
    QTest::ignoreMessage(
        QtWarningMsg,
        QRegularExpression(
            u".*RequestBuilder::clientCert\\(\\): empty chain, null certificate, or null key.*"_s));
    const auto result = m_client->get(QUrl(u"http://192.0.2.1/test"_s))
                            .clientCert({QSslCertificate{}}, QSslKey{})
                            .timeout(std::chrono::seconds(1))
                            .sync();
    QCOMPARE(result.code, HttpErrorCode::Timeout);
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
    CallbackReceiver receiver;
    const auto task
        = m_client->get(QUrl(u"http://192.0.2.1/test"_s))
              .timeout(std::chrono::seconds(30))
              .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
              .send();
    const auto copy = task; // 拷贝与原句柄指向同一在途请求
    copy.cancel();
    QTest::qWait(200);
    QCOMPARE(receiver.finishedCount, 0);
}

void HttpClientNoNetTest::syncIgnoresFinishedCallback()
{
    CallbackReceiver receiver;
    // sync() 的契约：结果经返回值交付，onFinished 被忽略——配置了即告警（仅此一条）；
    // onProgress 不在忽略之列，sync 阻塞期间正常派发、不告警
    //（派发契约由服务端套件 testSyncDownloadProgress 固化；
    //  本例为无网黑洞环境，进度无从产生，不做断言）
    QTest::ignoreMessage(
        QtWarningMsg, QRegularExpression(u".*RequestBuilder::sync\\(\\): onFinished ignored.*"_s));
    const auto result
        = m_client->get(QUrl(u"http://192.0.2.1/test"_s))
              .timeout(std::chrono::seconds(1))
              .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
              .onProgress(makeLifecycleCallback(&receiver, &CallbackReceiver::handleProgress))
              .sync();
    QCOMPARE(result.code, HttpErrorCode::Timeout);
    QCOMPARE(receiver.finishedCount, 0); // onFinished 仍被忽略：结果只经返回值交付
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
    CallbackReceiver receiver;
    auto builder = m_client->get(QUrl(u"http://192.0.2.1/test"_s));
    builder.timeout(std::chrono::seconds(1))
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished));
    builder.send();
    builder.send(); // 已消费：告警并忽略，不发起第二个请求
    QTRY_VERIFY_WITH_TIMEOUT(receiver.finishedCount == 1, 10000); // 首个请求恰好一次回调
    QTest::qWait(1500);                                           // 超出超时窗口后仍无第二次回调
    QCOMPARE(receiver.finishedCount, 1);

    auto syncBuilder = m_client->get(QUrl(u"http://192.0.2.1/test"_s));
    syncBuilder.timeout(std::chrono::seconds(1));
    (void) syncBuilder.sync(); // 首次：正常等待超时
    const auto reused = syncBuilder.sync();
    QCOMPARE(reused.code, HttpErrorCode::InvalidUse);
    QVERIFY(!reused.success());
    QVERIFY(!reused.message.isEmpty());
}

void HttpClientNoNetTest::builderMovedFromIsSafeNoOp()
{
    // 移动后的原对象（moved-from）：配置方法为安全空操作，
    // send() 告警并返回空句柄，sync() 返回 InvalidUse，均不崩溃
    auto builder = m_client->get(QUrl(u"http://192.0.2.1/test"_s));
    {
        RequestBuilder moved = std::move(builder); // 配置被移走，moved 立即析构
    }

    builder.timeout(std::chrono::seconds(1)); // no-op：不崩溃
    builder.basicAuth(u"user"_s, u"pass"_s);  // no-op：不崩溃、不写 host 表
    builder.send();                           // 告警并返回空句柄，不崩溃
    const auto result = builder.sync();       // moved-from：InvalidUse
    QCOMPARE(result.code, HttpErrorCode::InvalidUse);
    QVERIFY(!result.message.isEmpty());
}

// resultReady 扩展点与用户回调的顺序契约：扩展点先到且携带完整结果
//（超时路径派发，无需服务器）
void HttpClientNoNetTest::resultReadyRunsBeforeFinishedCallback()
{
    QStringList events;
    HttpResult readyResult;

    struct ProbeClient : HttpClient
    {
        QStringList *events = nullptr;
        HttpResult *readyResult = nullptr;

    protected:
        void resultReady(const HttpResult &result) override
        {
            events->append(u"resultReady"_s);
            *readyResult = result;
        }
    } client;

    client.events = &events;
    client.readyResult = &readyResult;

    client.get(QUrl(u"http://192.0.2.1/test"_s))
        .timeout(std::chrono::seconds(1))
        .onFinished([&events](const HttpResult &) { events.append(u"onFinished"_s); })
        .send();

    QTRY_VERIFY_WITH_TIMEOUT(events.size() == 2, 10000);
    QCOMPARE(events, (QStringList{u"resultReady"_s, u"onFinished"_s}));
    // 超时路径的结果同样携带动词
    QCOMPARE(readyResult.code, HttpErrorCode::Timeout);
    QCOMPARE(readyResult.method, QByteArray("GET"));
}

// 本地失败（文件打开失败）经事件循环派发：method 字段仍携带
void HttpClientNoNetTest::localFailureResultCarriesMethod()
{
    const auto blocker = m_tempDir.filePath(u"blocker2.txt"_s);
    {
        QFile file(blocker);
        QVERIFY(file.open(QIODevice::WriteOnly));
    }
    CallbackReceiver receiver;
    m_client->download(QUrl(u"http://192.0.2.1/file"_s), blocker + u"/sub.txt"_s)
        .onFinished(makeLifecycleCallback(&receiver, &CallbackReceiver::handleFinished))
        .send();
    QTRY_VERIFY_WITH_TIMEOUT(receiver.finishedCount == 1, 5000);
    QCOMPARE(receiver.lastResult.code, HttpErrorCode::FileError);
    QCOMPARE(receiver.lastResult.method, QByteArray("GET"));
}

void HttpClientNoNetTest::sensitiveRequestHeadersRedactedInLogs()
{
    // 敏感头脱敏（子串匹配：authorization/cookie/token/secret/api_key/apikey/api-key）。
    // 请求头日志在 prepare 期打印，不依赖服务器可达；debug 分类默认关闭，临时开启
    QLoggingCategory::setFilterRules(u"httpclient.debug=true"_s);
    {
        const std::lock_guard lock(logMutex);
        logBuffer.clear();
    }
    const auto oldHandler = qInstallMessageHandler(captureLog);
    const auto result = m_client->get(QUrl(u"http://192.0.2.1/test"_s))
                            .bearer("super-secret-token")
                            .header("X-Api-Key", "key-12345")
                            .header("X-Custom", "plain-value")
                            .timeout(std::chrono::seconds(1))
                            .sync();
    qInstallMessageHandler(oldHandler);
    QLoggingCategory::setFilterRules(u"httpclient.debug=false"_s);

    QString captured;
    {
        const std::lock_guard lock(logMutex);
        captured = logBuffer;
        logBuffer.clear();
    }
    QCOMPARE(result.code, HttpErrorCode::Timeout);
    QVERIFY2(!captured.contains(u"super-secret-token"_s), "bearer token leaked into log");
    QVERIFY2(!captured.contains(u"key-12345"_s), "api key leaked into log");
    // 格式无关断言：头名大小写/空格形态不做承诺，只验证脱敏结果与漏泄
    QVERIFY(captured.contains(u"***"_s));         // 敏感请求头已打码
    QVERIFY(captured.contains(u"plain-value"_s)); // 非敏感头值原样保留
}

QTEST_MAIN(HttpClientNoNetTest)
#include "httpclient_nonet_unittest.moc"
