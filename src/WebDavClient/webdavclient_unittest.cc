#include "webdavclient.hpp"

#include <LifecycleCallback/lifecyclecallback_qt.hpp>

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QScopedPointer>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;

// Integration tests: require the Flask DAV test server (unittests/dav_server.py)
// running on http://127.0.0.1:8001. All tests are skipped when it is unreachable.

namespace {

// dav_server.py 的 LOCK fixture 回显的固定令牌
constexpr auto kServerLockToken = "opaquelocktoken:urn:uuid:e71d4fa8-5d13-11e2-a65f-ff7ec9e8b68c";

} // namespace

class WebDavClientTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    // 词汇映射（/echo 回显断言：动词/头/体逐项核对）
    void testPropfindSendsVerbDepthAndBody();
    void testPropfindDepthValues();
    void testMkcolSendsVerb();
    void testCopySendsDestinationAndOverwrite();
    void testMoveSendsDestination();
    void testOptionsSendsVerb();
    void testLockSendsLockinfoBody();
    void testUnlockSendsLockTokenHeader();

    // 207（fixture 端点，经 check() 断言）
    void testCheckPropfind207YieldsResources();
    void testCheckMalformed207Fails();

    // 文件语义（/fs 真实目录行为）
    void testMkcolCreatesCollectionAndFailsOnExisting();
    void testMkcolParentMissingConflicts();
    void testCopyOverwriteFalsePreconditionFails();
    void testMoveRelocatesFile();

    // 锁（/fs + 服务器端 LOCKS 状态）
    void testLockStoresTokenAndAutoAttachesIf();
    void testPutLockedWithoutTokenIsLocked();
    void testUnlockReleasesLock();
    void testUnlockExplicitToken();
    // 顺序契约：lock 的 onFinished 回调内令牌须已入表
    void testTokenReadyInsideLockCallback();

    // If 注入与令牌表不变量
    void testCopyLockedSourceAutoAttachesIf(); // CustomOperation 分支（COPY 走 sendCustomRequest）
    void testExplicitIfHeaderWinsOverAutoInjection();
    void testReadOnlyVerbGetsNoIfHeader();
    void testUnlockWithoutTokenSendsNoHeader();
    void testFailedLockDoesNotOverwriteToken();    // LOCK 423：失败响应不污染既有令牌
    void testFailedUnlockKeepsToken();             // UNLOCK 400：失败响应不删除令牌
    void testRawBaseClassCallBypassesTokenTable(); // 基类原始调用边界（文档契约）

    // 继承冒烟：会话机制对 DAV 路径直接可用
    void testInheritedPutDownloadRoundTrip();

    // check()（动词感知的结果判定，/code/<int> 注入错误状态）
    void testCheckCleanTwoXxIsSuccess();
    void testCheckServerErrorMessages();
    void testCheckLocalErrorPassesMessage();
    void testCheckDownloadFileErrorPassthrough();

private:
    [[nodiscard]] static bool isServerAvailable();

    WebDavClient *m_dav = nullptr;
    QString m_base;
    QTemporaryDir m_tempDir;
    bool m_serverAvailable = false;
};

void WebDavClientTest::initTestCase()
{
    m_dav = new WebDavClient(this);
    m_base = u"http://127.0.0.1:8001"_s;
    QVERIFY(m_tempDir.isValid());
    m_serverAvailable = isServerAvailable();
    if (!m_serverAvailable) {
        qWarning() << "DAV test server not available at" << m_base
                   << "- skipping integration tests";
    }
}

void WebDavClientTest::cleanupTestCase()
{
    if (m_serverAvailable) {
        m_dav->post(QUrl(m_base + u"/__reset"_s)).sync();
    }
}

void WebDavClientTest::init()
{
    if (!m_serverAvailable) {
        QSKIP("DAV test server is not running");
    }
    // 每用例前清空服务器端 LOCKS 与文件根目录，保证用例间独立
    QVERIFY(m_dav->post(QUrl(m_base + u"/__reset"_s)).sync().success());
}

bool WebDavClientTest::isServerAvailable()
{
    HttpClient probe;
    const auto result = probe.get(QUrl(u"http://127.0.0.1:8001/health"_s))
                            .timeout(std::chrono::seconds(3))
                            .sync();
    return result.success();
}

// ── 词汇映射 ──

void WebDavClientTest::testPropfindSendsVerbDepthAndBody()
{
    const auto result = m_dav->propfind(QUrl(m_base + u"/echo"_s), WebDavClient::Depth::One).sync();
    QVERIFY(result.success());

    const auto obj = result.json.object();
    QCOMPARE(obj.value(u"method"_s).toString(), u"PROPFIND"_s);
    QCOMPARE(obj.value(u"depth"_s).toString(), u"1"_s);
    QCOMPARE(obj.value(u"content_type"_s).toString(), u"application/xml; charset=utf-8"_s);
    QVERIFY(obj.value(u"has_allprop"_s).toBool());
}

void WebDavClientTest::testPropfindDepthValues()
{
    struct
    {
        WebDavClient::Depth depth;
        QString expected;
    } cases[] = {
        {WebDavClient::Depth::Zero, u"0"_s},
        {WebDavClient::Depth::One, u"1"_s},
        {WebDavClient::Depth::Infinity, u"infinity"_s},
    };

    for (const auto &c : cases) {
        const auto result = m_dav->propfind(QUrl(m_base + u"/echo"_s), c.depth).sync();
        QVERIFY(result.success());
        QCOMPARE(result.json.object().value(u"depth"_s).toString(), c.expected);
    }
}

void WebDavClientTest::testMkcolSendsVerb()
{
    const auto result = m_dav->mkcol(QUrl(m_base + u"/echo"_s)).sync();
    QVERIFY(result.success());

    const auto obj = result.json.object();
    QCOMPARE(obj.value(u"method"_s).toString(), u"MKCOL"_s);
    QCOMPARE(obj.value(u"size"_s).toInt(), 0);
}

void WebDavClientTest::testCopySendsDestinationAndOverwrite()
{
    const auto result
        = m_dav->copy(QUrl(m_base + u"/echo"_s), QUrl(m_base + u"/fs/b.txt"_s)).sync();
    QVERIFY(result.success());

    const auto obj = result.json.object();
    QCOMPARE(obj.value(u"method"_s).toString(), u"COPY"_s);
    QCOMPARE(obj.value(u"destination"_s).toString(), m_base + u"/fs/b.txt"_s);
    // 默认与 RFC 4918 一致：Overwrite T
    QCOMPARE(obj.value(u"overwrite"_s).toString(), u"T"_s);

    const auto noOverwrite
        = m_dav->copy(QUrl(m_base + u"/echo"_s), QUrl(m_base + u"/fs/b.txt"_s), false).sync();
    QVERIFY(noOverwrite.success());
    QCOMPARE(noOverwrite.json.object().value(u"overwrite"_s).toString(), u"F"_s);
}

void WebDavClientTest::testMoveSendsDestination()
{
    const auto result
        = m_dav->move(QUrl(m_base + u"/echo"_s), QUrl(m_base + u"/fs/c.txt"_s)).sync();
    QVERIFY(result.success());

    const auto obj = result.json.object();
    QCOMPARE(obj.value(u"method"_s).toString(), u"MOVE"_s);
    QCOMPARE(obj.value(u"destination"_s).toString(), m_base + u"/fs/c.txt"_s);
}

void WebDavClientTest::testOptionsSendsVerb()
{
    const auto result = m_dav->options(QUrl(m_base + u"/echo"_s)).sync();
    QVERIFY(result.success());
    QCOMPARE(result.json.object().value(u"method"_s).toString(), u"OPTIONS"_s);
}

void WebDavClientTest::testLockSendsLockinfoBody()
{
    const auto result = m_dav->lock(QUrl(m_base + u"/echo"_s)).sync();
    QVERIFY(result.success());

    const auto obj = result.json.object();
    QCOMPARE(obj.value(u"method"_s).toString(), u"LOCK"_s);
    QCOMPARE(obj.value(u"content_type"_s).toString(), u"application/xml; charset=utf-8"_s);
    QVERIFY(obj.value(u"has_lockinfo"_s).toBool());
}

void WebDavClientTest::testUnlockSendsLockTokenHeader()
{
    const QUrl url(m_base + u"/echo"_s);
    m_dav->rememberLock(url, QByteArrayLiteral("tok-123"));

    const auto result = m_dav->unlock(url).sync();
    QVERIFY(result.success());

    const auto obj = result.json.object();
    QCOMPARE(obj.value(u"method"_s).toString(), u"UNLOCK"_s);
    // RFC 4918 要求 Lock-Token 值含尖括号
    QCOMPARE(obj.value(u"lock_token"_s).toString(), u"<tok-123>"_s);

    const auto explicitToken = m_dav->unlock(url, "tok-456").sync();
    QVERIFY(explicitToken.success());
    QCOMPARE(explicitToken.json.object().value(u"lock_token"_s).toString(), u"<tok-456>"_s);
}

// ── 207（经 check()）──

void WebDavClientTest::testCheckPropfind207YieldsResources()
{
    const auto checked = WebDavClient::check(m_dav->propfind(QUrl(m_base + u"/dav/dir/"_s)).sync());

    QVERIFY(checked);
    QCOMPARE(checked.code, DavErrorCode::Multistatus);
    QCOMPARE(checked.resources.size(), 2);

    const auto &dir = checked.resources.at(0);
    QCOMPARE(dir.href, QUrl(m_base + u"/dav/dir/"_s));
    QVERIFY(dir.isCollection);
    QCOMPARE(dir.displayName, u"dir"_s);
    QCOMPARE(dir.status, 200);
    QVERIFY(dir.lastModified.isValid());

    const auto &file = checked.resources.at(1);
    QCOMPARE(file.href, QUrl(m_base + u"/dav/dir/hello.txt"_s));
    QVERIFY(!file.isCollection);
    QCOMPARE(file.contentLength, qint64(13));
    QCOMPARE(file.contentType, u"text/plain"_s);
    QCOMPARE(file.displayName, u"hello.txt"_s);
    QVERIFY(file.etag.isEmpty()); // 404 propstat 的属性不进结果
    QVERIFY(file.lastModified.isValid());
}

void WebDavClientTest::testCheckMalformed207Fails()
{
    const auto checked
        = WebDavClient::check(m_dav->propfind(QUrl(m_base + u"/malformed"_s)).sync());

    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Multistatus);
    QVERIFY(checked.message.contains(u"Malformed"_s));
    QVERIFY(checked.resources.isEmpty());
}

// ── 文件语义 ──

void WebDavClientTest::testMkcolCreatesCollectionAndFailsOnExisting()
{
    const QUrl dir(m_base + u"/fs/coll"_s);
    QVERIFY(m_dav->mkcol(dir).sync().success());

    const auto again = m_dav->mkcol(dir).sync();
    QVERIFY(!again.success());
    QCOMPARE(again.status, 405);
}

void WebDavClientTest::testMkcolParentMissingConflicts()
{
    const auto result = m_dav->mkcol(QUrl(m_base + u"/fs/no/such/dir"_s)).sync();
    QVERIFY(!result.success());
    QCOMPARE(result.status, 409);
    QCOMPARE(WebDavClient::check(result).code, DavErrorCode::Conflict);
}

void WebDavClientTest::testCopyOverwriteFalsePreconditionFails()
{
    const QUrl src(m_base + u"/fs/src.txt"_s);
    const QUrl dst(m_base + u"/fs/dst.txt"_s);
    QVERIFY(m_dav->upload(src).putData("s").sync().success());
    QVERIFY(m_dav->upload(dst).putData("d").sync().success());

    const auto result = m_dav->copy(src, dst, false).sync();
    QVERIFY(!result.success());
    QCOMPARE(result.status, 412);
    QCOMPARE(WebDavClient::check(result).code, DavErrorCode::PreconditionFailed);
    // Overwrite=T 则覆盖成功
    QVERIFY(m_dav->copy(src, dst, true).sync().success());
}

void WebDavClientTest::testMoveRelocatesFile()
{
    const QUrl from(m_base + u"/fs/m1.txt"_s);
    const QUrl to(m_base + u"/fs/m2.txt"_s);
    QVERIFY(m_dav->upload(from).putData("payload").sync().success());

    QVERIFY(m_dav->move(from, to).sync().success());

    const auto moved = m_dav->get(to).sync();
    QVERIFY(moved.success());
    QCOMPARE(moved.body, "payload");

    const auto gone = m_dav->get(from).sync();
    QVERIFY(!gone.success());
    QCOMPARE(gone.status, 404);
}

// ── 锁 ──

void WebDavClientTest::testLockStoresTokenAndAutoAttachesIf()
{
    const QUrl res(m_base + u"/fs/locked.txt"_s);
    QVERIFY(m_dav->upload(res).putData("v1").sync().success());

    QVERIFY(m_dav->lock(res).sync().success());
    // LOCK 200 响应的令牌自动入表
    QVERIFY(m_dav->hasLock(res));
    QCOMPARE(m_dav->lockToken(res), QByteArray(kServerLockToken));

    // 锁定资源的 PUT：createRequest 自动注入 If 头，服务器放行
    const auto put = m_dav->upload(res).putData("v2").sync();
    QVERIFY(put.success());

    const auto got = m_dav->get(res).sync();
    QVERIFY(got.success());
    QCOMPARE(got.body, "v2");
}

void WebDavClientTest::testPutLockedWithoutTokenIsLocked()
{
    const QUrl res(m_base + u"/fs/x.txt"_s);
    QVERIFY(m_dav->upload(res).putData("seed").sync().success());
    QVERIFY(m_dav->lock(res).sync().success());

    // 另一 client 无令牌：PUT 被服务器 423 拒绝
    WebDavClient other;
    const auto result = other.upload(res).putData("nope").sync();
    QVERIFY(!result.success());
    QCOMPARE(result.status, 423);
    QCOMPARE(WebDavClient::check(result).code, DavErrorCode::Locked);
}

void WebDavClientTest::testUnlockReleasesLock()
{
    const QUrl res(m_base + u"/fs/u.txt"_s);
    QVERIFY(m_dav->upload(res).putData("seed").sync().success());
    QVERIFY(m_dav->lock(res).sync().success());

    const auto result = m_dav->unlock(res).sync();
    QVERIFY(result.success());
    QCOMPARE(result.status, 204);
    QVERIFY(!m_dav->hasLock(res));

    // 服务器端锁已释放：其他 client 可直接写
    WebDavClient other;
    QVERIFY(other.upload(res).putData("after-unlock").sync().success());
}

void WebDavClientTest::testUnlockExplicitToken()
{
    const QUrl res(m_base + u"/fs/e.txt"_s);
    QVERIFY(m_dav->upload(res).putData("seed").sync().success());
    QVERIFY(m_dav->lock(res).sync().success());

    // 显式令牌版本不经令牌表
    WebDavClient other;
    const auto result = other.unlock(res, kServerLockToken).sync();
    QVERIFY(result.success());
    QCOMPARE(result.status, 204);

    QVERIFY(m_dav->upload(res).putData("after-unlock").sync().success());
}

void WebDavClientTest::testTokenReadyInsideLockCallback()
{
    const QUrl res(m_base + u"/fs/order.txt"_s);
    QVERIFY(m_dav->upload(res).putData("seed").sync().success());

    // lock 的 onFinished 回调内立即 PUT：resultReady 先于用户回调派发，
    // 令牌已入表、If 已可注入（否则服务器以 423 拒绝该 PUT）——守护顺序契约
    bool putSucceeded = false;
    QEventLoop loop;
    m_dav->lock(res)
        .onFinished([&](const HttpResult &) {
            putSucceeded = m_dav->upload(res).putData("in-callback").sync().success();
            loop.quit();
        })
        .send();
    loop.exec();
    QVERIFY(putSucceeded);
    QVERIFY(m_dav->hasLock(res));
}

// ── If 注入与令牌表不变量 ──

void WebDavClientTest::testCopyLockedSourceAutoAttachesIf()
{
    // COPY 经 sendCustomRequest 走 CustomOperation 分支：源被锁时 If 自动
    // 注入，服务器放行
    const QUrl src(m_base + u"/fs/locked-src.txt"_s);
    const QUrl dst(m_base + u"/fs/locked-dst.txt"_s);
    QVERIFY(m_dav->upload(src).putData("data").sync().success());
    QVERIFY(m_dav->lock(src).sync().success());
    QVERIFY(m_dav->hasLock(src));

    // 无令牌的第二个 client：COPY 被服务器 423 拒绝（对照组，证明锁生效）
    WebDavClient other;
    QVERIFY(!other.copy(src, dst).sync().success());

    // 持锁 client：If 注入使 COPY 放行
    QVERIFY(m_dav->copy(src, dst).sync().success());
    const auto got = m_dav->get(dst).sync();
    QVERIFY(got.success());
    QCOMPARE(got.body, "data");
}

void WebDavClientTest::testExplicitIfHeaderWinsOverAutoInjection()
{
    // 文档契约"显式 If 优先"：rememberLock 后链上 header("If", ...) 设置的
    // 值必须原样到达服务器，不被自动注入覆盖
    const QUrl echo(m_base + u"/echo"_s);
    m_dav->rememberLock(echo, QByteArrayLiteral("auto-token"));

    const auto result = m_dav->copy(echo, QUrl(m_base + u"/fs/any.txt"_s))
                            .header("If", "(<explicit-token>)")
                            .sync();
    QVERIFY(result.success());
    QCOMPARE(result.json.object().value(u"if"_s).toString(), u"(<explicit-token>)"_s);

    // 对照：不显式设置时自动注入的值到达服务器
    const auto autoResult = m_dav->copy(echo, QUrl(m_base + u"/fs/any.txt"_s)).sync();
    QVERIFY(autoResult.success());
    QCOMPARE(autoResult.json.object().value(u"if"_s).toString(), u"(<auto-token>)"_s);
    m_dav->forgetLock(echo);
}

void WebDavClientTest::testReadOnlyVerbGetsNoIfHeader()
{
    // 文档契约"只读动词不注入"：PROPFIND 不携带 If 令牌
    const QUrl echo(m_base + u"/echo"_s);
    m_dav->rememberLock(echo, QByteArrayLiteral("tok"));

    const auto result = m_dav->propfind(echo).sync();
    QVERIFY(result.success());
    QVERIFY(result.json.object().value(u"if"_s).toString().isEmpty());

    m_dav->forgetLock(echo);
}

void WebDavClientTest::testUnlockWithoutTokenSendsNoHeader()
{
    // 文档契约"表内无令牌时告警并照常发送"：无 Lock-Token 头
    const auto result = m_dav->unlock(QUrl(m_base + u"/echo"_s)).sync();
    QVERIFY(result.success());
    QVERIFY(result.json.object().value(u"lock_token"_s).toString().isEmpty());
}

void WebDavClientTest::testFailedLockDoesNotOverwriteToken()
{
    // /code/423 对 LOCK 返回 423：失败响应不得覆盖既有令牌，也不得新写入
    const QUrl res(m_base + u"/fs/fail-lock.txt"_s);
    QVERIFY(m_dav->upload(res).putData("seed").sync().success());
    QVERIFY(m_dav->lock(res).sync().success());
    QCOMPARE(m_dav->lockToken(res), QByteArray(kServerLockToken));

    // 服务器锁定冲突路径（/code/423）：LOCK 返回 423，表内令牌保持原值
    const QUrl conflict(m_base + u"/code/423"_s);
    QVERIFY(!m_dav->lock(conflict).sync().success());
    QVERIFY(!m_dav->hasLock(conflict));                            // 失败 LOCK 不新写入
    QCOMPARE(m_dav->lockToken(res), QByteArray(kServerLockToken)); // 既有令牌不被覆盖
}

void WebDavClientTest::testFailedUnlockKeepsToken()
{
    // /code/400 对 UNLOCK 返回 400：失败响应不得删除表内令牌
    const QUrl res(m_base + u"/fs/fail-unlock.txt"_s);
    QVERIFY(m_dav->upload(res).putData("seed").sync().success());
    QVERIFY(m_dav->lock(res).sync().success());
    QVERIFY(m_dav->hasLock(res));

    const QUrl bad(m_base + u"/code/400"_s);
    m_dav->rememberLock(bad, QByteArrayLiteral("tok"));
    QVERIFY(!m_dav->unlock(bad).sync().success());

    QVERIFY(m_dav->hasLock(bad)); // 失败 UNLOCK 不删表
    QVERIFY(m_dav->hasLock(res)); // 无关条目不受影响
}

void WebDavClientTest::testRawBaseClassCallBypassesTokenTable()
{
    // HttpClient 文档契约：基类原始调用不参与回调管理——裸 sendCustomRequest
    // 发出的 LOCK 不经 resultReady，令牌不入表
    QNetworkRequest request;
    request.setUrl(QUrl(m_base + u"/echo"_s));
    request.setHeader(QNetworkRequest::ContentTypeHeader, u"application/xml"_s);
    QScopedPointer<QNetworkReply> reply(m_dav->sendCustomRequest(request, "LOCK"));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    reply->deleteLater();

    QVERIFY(!m_dav->hasLock(QUrl(m_base + u"/echo"_s)));
}

// ── 继承冒烟 ──

void WebDavClientTest::testInheritedPutDownloadRoundTrip()
{
    const QUrl res(m_base + u"/fs/file.txt"_s);
    QVERIFY(m_dav->upload(res).putData("round trip body").sync().success());

    const QString target = m_tempDir.filePath(u"file.txt"_s);
    const auto download = m_dav->download(res, target).sync();
    QVERIFY(download.success());

    QFile file(target);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(file.readAll()), u"round trip body"_s);
}

// ── check() ──

void WebDavClientTest::testCheckCleanTwoXxIsSuccess()
{
    const auto result = m_dav->options(QUrl(m_base + u"/echo"_s)).sync();
    QVERIFY(result.success());
    QCOMPARE(result.status, 200);

    const auto checked = WebDavClient::check(result);
    QVERIFY(checked);
    QCOMPARE(checked.code, DavErrorCode::NoError);
    QVERIFY(checked.message.isEmpty());
}

void WebDavClientTest::testCheckServerErrorMessages()
{
    // 措辞表（RFC 4918 §9，405/409/423 按动词消歧）+ 未命中透传 errorString
    struct
    {
        QByteArray verb;
        int status;
        DavErrorCode code;
        QString fragment;
        bool passthrough = false;
    } cases[] = {
        {"MKCOL", 409, DavErrorCode::Conflict, u"parent collection"_s},
        {"MKCOL", 405, DavErrorCode::Other, u"already exists"_s},
        {"PUT", 405, DavErrorCode::Other, u"method not allowed"_s},
        {"COPY", 412, DavErrorCode::PreconditionFailed, u"Overwrite=F"_s},
        {"LOCK", 423, DavErrorCode::Locked, u"lock"_s},
        {"GET", 403, DavErrorCode::Other, u"forbidden"_s},
        {"PUT", 507, DavErrorCode::InsufficientStorage, u"storage"_s},
        {"PROPFIND", 415, DavErrorCode::UnsupportedMediaType, u"media type"_s},
        {"COPY", 502, DavErrorCode::BadGateway, u"unreachable"_s},
        {"GET", 500, DavErrorCode::Other, {}, true},
    };

    for (const auto &c : cases) {
        const auto result
            = m_dav->get(QUrl(m_base + u"/code/%1"_s.arg(c.status))).method(c.verb).sync();
        QVERIFY(!result.success());
        QCOMPARE(result.status, c.status);

        const auto checked = WebDavClient::check(result);
        QVERIFY(!checked);
        QCOMPARE(checked.code, c.code);
        if (c.passthrough) {
            // 未命中措辞表：透传 QNAM errorString（含 "server replied: ..."）
            QCOMPARE(checked.message, result.message);
            QVERIFY(!checked.message.isEmpty());
        } else {
            QVERIFY(checked.message.contains(c.fragment, Qt::CaseInsensitive));
        }
    }
}

void WebDavClientTest::testCheckLocalErrorPassesMessage()
{
    // 二次使用构建器：InvalidUse（status == 0），message 透传
    auto builder = m_dav->get(QUrl(m_base + u"/health"_s));
    QVERIFY(builder.sync().success());
    const auto reused = builder.sync();
    QCOMPARE(reused.code, HttpErrorCode::InvalidUse);
    QCOMPARE(reused.status, 0);

    auto checked = WebDavClient::check(reused);
    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Other);
    QCOMPARE(checked.message, reused.message);
    QVERIFY(!checked.message.isEmpty());

    // 连接层失败（拒绝连接的本地端口）：同为 status == 0 透传路径
    const auto refused
        = m_dav->get(QUrl(u"http://127.0.0.1:1/x"_s)).timeout(std::chrono::seconds(5)).sync();
    QVERIFY(!refused.success());
    QCOMPARE(refused.status, 0);

    checked = WebDavClient::check(refused);
    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Other);
    QCOMPARE(checked.message, refused.message);
    QVERIFY(!checked.message.isEmpty());
}

void WebDavClientTest::testCheckDownloadFileErrorPassthrough()
{
    // 2xx 状态行已到 + 下载落盘失败（目标被同名目录占用）：code=FileError
    // 覆盖网络结果，check 须判失败并透传 FileError 消息——只看 200 状态码
    // 会把落盘失败误判为成功
    const QUrl res(m_base + u"/fs/dl-err.txt"_s);
    QVERIFY(m_dav->upload(res).putData("payload").sync().success());

    const QString target = m_tempDir.filePath(u"occupied"_s);
    QVERIFY(QDir().mkpath(target));

    const auto result = m_dav->download(res, target).sync();
    QCOMPARE(result.code, HttpErrorCode::FileError);
    QCOMPARE(result.status, 200);

    const auto checked = WebDavClient::check(result);
    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Other);
    QCOMPARE(checked.message, result.message);
    QVERIFY(!checked.message.isEmpty());
}

QTEST_MAIN(WebDavClientTest)
#include "webdavclient_unittest.moc"
