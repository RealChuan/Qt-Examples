#include "webdavclient.hpp"

#include <QTest>

#include <algorithm>

using namespace Qt::StringLiterals;

// WsgiDAV 真实服务器符合性测试（RFC 4918）：unittests/wsgidav_server.py
// 运行于 127.0.0.1:8002（pip install wsgidav 后启动），不可达时整批跳过。
// 与 dav_server.py（Flask 故障注入）互补：本套件验证标准行为——真实 207
// XML 结构、真实锁管理器（Class 2）、真实状态码语义；异常路径（部分失败
// 207 / 畸形 XML / 状态码注入）由 mock 套件覆盖。

class WebDavConformanceTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void testPropfindRealMultistatus();
    void testMkcolSemantics();
    void testPutGetRoundTrip();
    void testCopyMoveSemantics();
    void testDeleteSemantics();
    void testOptionsDavHeader();
    void testLockAgainstRealLockManager();
    void testPercentEncodedHrefs();

private:
    [[nodiscard]] static bool isServerAvailable();
    void deleteTree(const QString &path);

    WebDavClient *m_dav = nullptr;
    QString m_base;
    QString m_root;
    bool m_serverAvailable = false;
};

void WebDavConformanceTest::initTestCase()
{
    m_dav = new WebDavClient(this);
    m_base = u"http://127.0.0.1:8002"_s;
    m_root = m_base + u"/suite"_s;
    m_serverAvailable = isServerAvailable();
    if (!m_serverAvailable) {
        qWarning() << "WsgiDAV conformance server not available at" << m_base
                   << "- skipping (pip install wsgidav; python unittests/wsgidav_server.py)";
        return;
    }
    deleteTree(m_root); // 上次运行残留，best-effort
    QVERIFY(m_dav->mkcol(QUrl(m_root)).sync().success());
}

void WebDavConformanceTest::cleanupTestCase()
{
    if (m_serverAvailable) {
        deleteTree(m_root);
    }
}

void WebDavConformanceTest::init()
{
    if (!m_serverAvailable) {
        QSKIP("WsgiDAV conformance server is not running");
    }
}

bool WebDavConformanceTest::isServerAvailable()
{
    WebDavClient probe;
    const auto result
        = probe.options(QUrl(u"http://127.0.0.1:8002/"_s)).timeout(std::chrono::seconds(3)).sync();
    return result.success() && !result.header("DAV").isEmpty();
}

void WebDavConformanceTest::deleteTree(const QString &path)
{
    // RFC 4918 §9.6.1：集合 DELETE 须携带 Depth: infinity
    m_dav->del(QUrl(path)).header("Depth", "infinity").sync();
}

// ── PROPFIND：真实 207 结构 ──

void WebDavConformanceTest::testPropfindRealMultistatus()
{
    const QString dir = m_root + u"/tree"_s;
    QVERIFY(m_dav->mkcol(QUrl(dir)).sync().success());
    QVERIFY(m_dav->upload(QUrl(dir + u"/file.txt"_s)).putData("hello world").sync().success());
    QVERIFY(m_dav->mkcol(QUrl(dir + u"/sub"_s)).sync().success());

    const auto checked
        = WebDavClient::check(m_dav->propfind(QUrl(dir + u"/"_s), WebDavClient::Depth::One).sync());
    QVERIFY(checked);
    QCOMPARE(checked.code, DavErrorCode::Multistatus);
    QCOMPARE(checked.resources.size(), 3); // 请求的集合自身 + 两个直接子资源

    const auto findByPath = [&checked](QStringView suffix, bool collection) {
        return std::ranges::find_if(checked.resources, [&](const DavResource &r) {
            return r.isCollection == collection && r.href.path().endsWith(suffix);
        });
    };

    QVERIFY(findByPath(u"/tree/"_s, true) != checked.resources.cend());
    QVERIFY(findByPath(u"/sub/"_s, true) != checked.resources.cend()); // 集合 href 带尾斜杠

    const auto file = findByPath(u"/file.txt"_s, false);
    QVERIFY(file != checked.resources.cend());
    QCOMPARE(file->contentLength, qint64(11));
    // 媒体类型可带参数（如 WsgiDAV 回 "text/plain; charset=utf-8"）：
    // 验证主类型而非整串相等
    QVERIFY(file->contentType.startsWith(u"text/plain"_s));
    QVERIFY(file->lastModified.isValid()); // 真实服务器回 IMF-fixdate
    QCOMPARE(file->status, 200);
    QVERIFY(!file->failedProps);

    // Depth 0：仅目标资源自身
    const auto zero = WebDavClient::check(
        m_dav->propfind(QUrl(dir + u"/file.txt"_s), WebDavClient::Depth::Zero).sync());
    QVERIFY(zero);
    QCOMPARE(zero.resources.size(), 1);
    QCOMPARE(zero.resources.at(0).contentLength, qint64(11));
}

// ── 状态码语义 ──

void WebDavConformanceTest::testMkcolSemantics()
{
    const QUrl dir(m_root + u"/mk"_s);
    QCOMPARE(m_dav->mkcol(dir).sync().status, 201);

    // 已存在：405（check 措辞按动词消歧）
    const auto existing = m_dav->mkcol(dir).sync();
    QCOMPARE(existing.status, 405);
    const auto checked = WebDavClient::check(existing);
    QVERIFY(!checked);
    QVERIFY(checked.message.contains(u"already exists"_s));

    // 父集合缺失：409
    const auto orphan = m_dav->mkcol(QUrl(m_root + u"/no/such/dir"_s)).sync();
    QCOMPARE(orphan.status, 409);
    QCOMPARE(WebDavClient::check(orphan).code, DavErrorCode::Conflict);
}

void WebDavConformanceTest::testPutGetRoundTrip()
{
    const QUrl file(m_root + u"/roundtrip.txt"_s);
    QCOMPARE(m_dav->upload(file).putData("conformance payload").sync().status, 201);

    const auto got = m_dav->get(file).sync();
    QVERIFY(got.success());
    QCOMPARE(got.body, "conformance payload");
}

void WebDavConformanceTest::testCopyMoveSemantics()
{
    const QUrl src(m_root + u"/cm-src.txt"_s);
    const QUrl dst(m_root + u"/cm-dst.txt"_s);
    QVERIFY(m_dav->upload(src).putData("cm").sync().success());
    QVERIFY(m_dav->upload(dst).putData("old").sync().success());

    // Overwrite=F 撞已有目标：412，且目标原内容未被破坏
    const auto clash = m_dav->copy(src, dst, false).sync();
    QCOMPARE(clash.status, 412);
    QCOMPARE(WebDavClient::check(clash).code, DavErrorCode::PreconditionFailed);
    QCOMPARE(m_dav->get(dst).sync().body, "old");

    // Overwrite=T 覆盖
    QVERIFY(m_dav->copy(src, dst, true).sync().success());
    QCOMPARE(m_dav->get(dst).sync().body, "cm");

    // MOVE：源消失
    const QUrl moved(m_root + u"/cm-moved.txt"_s);
    QVERIFY(m_dav->move(src, moved).sync().success());
    QCOMPARE(m_dav->get(src).sync().status, 404);
    QCOMPARE(m_dav->get(moved).sync().body, "cm");
}

void WebDavConformanceTest::testDeleteSemantics()
{
    const QUrl file(m_root + u"/del.txt"_s);
    QVERIFY(m_dav->upload(file).putData("x").sync().success());
    QCOMPARE(m_dav->del(file).sync().status, 204);

    // 重复删除：404
    QCOMPARE(m_dav->del(file).sync().status, 404);

    const auto gone = m_dav->get(file).sync();
    QCOMPARE(gone.status, 404);
    const auto checked = WebDavClient::check(gone);
    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Other);
    QVERIFY(!checked.message.isEmpty()); // 透传 QNAM errorString（含状态行原因）
}

void WebDavConformanceTest::testOptionsDavHeader()
{
    const auto result = m_dav->options(QUrl(m_base + u"/"_s)).sync();
    QVERIFY(result.success());
    const auto dav = result.header("DAV");
    QVERIFY(!dav.isEmpty());
    QVERIFY(dav.contains("1"));
    QVERIFY(dav.contains("2")); // WsgiDAV 实现 Class 2（锁）
}

// ── 锁：真实 LockManager ──

void WebDavConformanceTest::testLockAgainstRealLockManager()
{
    const QUrl file(m_root + u"/locked.txt"_s);
    QVERIFY(m_dav->upload(file).putData("v1").sync().success());

    // LOCK：真实 lockdiscovery，令牌自动入表
    QVERIFY(m_dav->lock(file).sync().success());
    QVERIFY(m_dav->hasLock(file));
    QVERIFY(!m_dav->lockToken(file).isEmpty());

    // 他端无令牌写入：423
    WebDavClient other;
    const auto rejected = other.upload(file).putData("nope").sync();
    QCOMPARE(rejected.status, 423);
    QCOMPARE(WebDavClient::check(rejected).code, DavErrorCode::Locked);

    // 持锁端写入：If 自动注入放行
    QVERIFY(m_dav->upload(file).putData("v2").sync().success());
    QCOMPARE(m_dav->get(file).sync().body, "v2");

    // UNLOCK 后他端可写
    QCOMPARE(m_dav->unlock(file).sync().status, 204);
    QVERIFY(!m_dav->hasLock(file));
    QVERIFY(other.upload(file).putData("v3").sync().success());
}

// ── href 形态 ──

void WebDavConformanceTest::testPercentEncodedHrefs()
{
    const QString dir = m_root + u"/my dir"_s;
    QVERIFY(m_dav->mkcol(QUrl(dir)).sync().success());
    QVERIFY(m_dav->upload(QUrl(dir + u"/my file.txt"_s)).putData("x").sync().success());

    const auto checked
        = WebDavClient::check(m_dav->propfind(QUrl(dir + u"/"_s), WebDavClient::Depth::One).sync());
    QVERIFY(checked);
    QCOMPARE(checked.resources.size(), 2);

    const auto it = std::ranges::find_if(checked.resources, [](const DavResource &r) {
        return !r.isCollection && r.href.path().endsWith(u"my file.txt"_s);
    });
    QVERIFY(it != checked.resources.cend());
    // 服务器回显的百分号编码形态被保留（真实服务器常态）
    QVERIFY(it->href.toString(QUrl::FullyEncoded).contains(u"%20"_s));
    QCOMPARE(it->href.fileName(), u"my file.txt"_s);
}

QTEST_MAIN(WebDavConformanceTest)
#include "webdavclient_conformance_unittest.moc"
