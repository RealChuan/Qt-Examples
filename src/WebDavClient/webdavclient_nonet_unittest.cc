#include "webdavclient.hpp"

#include <QDateTime>
#include <QTest>
#include <QTimeZone>

using namespace Qt::StringLiterals;

// 无网络测试：check() 判定与 207 解析 / 锁令牌表。
// HttpResult 均为手工构造的 fixture，不发起任何请求。

namespace {

HttpResult
makeResult(int status, const QByteArray &body, const QUrl &url, QByteArray method = "PROPFIND")
{
    HttpResult result;
    result.url = url;
    result.status = status;
    result.body = body;
    result.method = std::move(method);
    // code 只需令 success() 与状态码一致：check() 不读具体网络错误值
    result.code
        = status >= 200 && status < 300 ? HttpErrorCode::NoError : HttpErrorCode::InvalidUse;
    return result;
}

} // namespace

class WebDavClientNoNetTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // check()：成功路径
    void checkTwoXxSucceeds();
    void checkPropfind207YieldsResources();
    void checkOtherVerb207AllOkSucceeds();

    // check()：207 解析分支
    void check207ResolvesRelativeHref();
    void check207KeepsAbsoluteUrlHref();
    void check207KeepsPercentEncodedHref();
    void check207ResponseStatusLine();
    void check207SkipsUnknownElementsAndInvalidProps();
    void check207MixedPropstatFails();
    void check207MissingHrefFails();
    void check207MalformedBodyFails();
    void check207EmptyBodyFails();
    void checkCopy207CountsFailures();

    // check()：错误分类与措辞
    void checkErrorClassification();
    void checkVerbDisambiguatedMessages();
    void checkUnmappedStatusPassesMessageThrough();
    void checkLocalErrorPassesMessage();

    // 锁令牌表
    void lockStoreSemantics();
    void lockStoreKeysAreExact();

    // Q_ENUM 反射
    void depthEnumReflection();

private:
    [[nodiscard]] static QDateTime utc(qint16 y, qint8 m, qint8 d, qint8 h, qint8 min);

    WebDavClient *m_dav = nullptr;
};

void WebDavClientNoNetTest::initTestCase()
{ m_dav = new WebDavClient(this); }

QDateTime WebDavClientNoNetTest::utc(qint16 y, qint8 m, qint8 d, qint8 h, qint8 min)
{ return {QDate(y, m, d), QTime(h, min), QTimeZone::UTC}; }

// ── 成功路径 ──

void WebDavClientNoNetTest::checkTwoXxSucceeds()
{
    for (const int status : {200, 201, 204}) {
        const auto checked = WebDavClient::check(makeResult(status, {}, QUrl()));
        QVERIFY(checked);
        QCOMPARE(checked.code, DavErrorCode::NoError);
        QVERIFY(checked.message.isEmpty());
        QVERIFY(checked.resources.isEmpty());
    }
}

void WebDavClientNoNetTest::checkPropfind207YieldsResources()
{
    // propstat 子元素为 RFC 4918 §14.24 标准序 (prop, status)
    const auto body
        = QByteArrayLiteral("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                            "<D:multistatus xmlns:D=\"DAV:\">"
                            "<D:response><D:href>/dav/dir/</D:href>"
                            "<D:propstat><D:prop>"
                            "<D:displayname>dir</D:displayname>"
                            "<D:resourcetype><D:collection/></D:resourcetype>"
                            "<D:getlastmodified>Tue, 13 Oct 2026 08:00:00 GMT</D:getlastmodified>"
                            "</D:prop><D:status>HTTP/1.1 200 OK</D:status></D:propstat>"
                            "</D:response>"
                            "<D:response><D:href>/dav/dir/hello.txt</D:href>"
                            "<D:propstat><D:prop>"
                            "<D:displayname>hello.txt</D:displayname>"
                            "<D:resourcetype/>"
                            "<D:getcontentlength>13</D:getcontentlength>"
                            "<D:getcontenttype>text/plain</D:getcontenttype>"
                            "<D:getlastmodified>Tue, 13 Oct 2026 08:30:00 GMT</D:getlastmodified>"
                            "</D:prop><D:status>HTTP/1.1 200 OK</D:status></D:propstat>"
                            "<D:propstat><D:prop><D:getetag/></D:prop>"
                            "<D:status>HTTP/1.1 404 Not Found</D:status></D:propstat>"
                            "</D:response></D:multistatus>");
    const auto checked
        = WebDavClient::check(makeResult(207, body, QUrl(u"http://example.org/dav/dir/"_s)));

    QVERIFY(checked);
    QCOMPARE(checked.code, DavErrorCode::Multistatus);
    QCOMPARE(checked.resources.size(), 2);

    const auto &dir = checked.resources.at(0);
    QCOMPARE(dir.href, QUrl(u"http://example.org/dav/dir/"_s));
    QVERIFY(dir.isCollection);
    QCOMPARE(dir.displayName, u"dir"_s);
    QCOMPARE(dir.contentLength, -1);
    QCOMPARE(dir.status, 200);
    QVERIFY(!dir.failedProps);
    QCOMPARE(dir.lastModified, utc(2026, 10, 13, 8, 0));

    const auto &file = checked.resources.at(1);
    QCOMPARE(file.href, QUrl(u"http://example.org/dav/dir/hello.txt"_s));
    QVERIFY(!file.isCollection);
    QCOMPARE(file.displayName, u"hello.txt"_s);
    QCOMPARE(file.contentLength, qint64(13));
    QCOMPARE(file.contentType, u"text/plain"_s);
    QVERIFY(file.etag.isEmpty()); // 404 propstat 的属性不进结果
    QVERIFY(file.failedProps);    // 属性缺失信号：不代表资源不存在
    QCOMPARE(file.lastModified, utc(2026, 10, 13, 8, 30));
}

void WebDavClientNoNetTest::checkOtherVerb207AllOkSucceeds()
{
    // PROPPATCH 恒回 207（RFC 4918 §9.2）：全 2xx propstat 即成功；
    // 两个 2xx propstat 各携带不同属性：掩码合并后均保留
    const auto body
        = QByteArrayLiteral("<D:multistatus xmlns:D=\"DAV:\"><D:response><D:href>/a.txt</D:href>"
                            "<D:propstat><D:prop><D:displayname>a</D:displayname></D:prop>"
                            "<D:status>HTTP/1.1 200 OK</D:status></D:propstat>"
                            "<D:propstat><D:prop><D:getetag>\"v1\"</D:getetag></D:prop>"
                            "<D:status>HTTP/1.1 200 OK</D:status></D:propstat>"
                            "</D:response></D:multistatus>");
    const auto checked
        = WebDavClient::check(makeResult(207, body, QUrl(u"http://example.org/"_s), "PROPPATCH"));

    QVERIFY(checked);
    QCOMPARE(checked.code, DavErrorCode::Multistatus);
    QCOMPARE(checked.resources.size(), 1);
    QCOMPARE(checked.resources.at(0).displayName, u"a"_s);
    QCOMPARE(checked.resources.at(0).etag, u"\"v1\""_s);
    QVERIFY(!checked.resources.at(0).failedProps);
}

// ── 207 解析分支 ──

void WebDavClientNoNetTest::check207ResolvesRelativeHref()
{
    const auto body
        = QByteArrayLiteral("<D:multistatus xmlns:D=\"DAV:\"><D:response><D:href>hello.txt</D:href>"
                            "</D:response></D:multistatus>");
    const auto checked
        = WebDavClient::check(makeResult(207, body, QUrl(u"http://example.org/dav/dir/"_s)));

    QVERIFY(checked);
    QCOMPARE(checked.resources.size(), 1);
    QCOMPARE(checked.resources.at(0).href, QUrl(u"http://example.org/dav/dir/hello.txt"_s));
}

void WebDavClientNoNetTest::check207KeepsAbsoluteUrlHref()
{
    const auto body = QByteArrayLiteral("<D:multistatus xmlns:D=\"DAV:\"><D:response>"
                                        "<D:href>https://other.example/x</D:href>"
                                        "</D:response></D:multistatus>");
    const auto checked
        = WebDavClient::check(makeResult(207, body, QUrl(u"http://example.org/dav/dir/"_s)));

    QVERIFY(checked);
    QCOMPARE(checked.resources.at(0).href, QUrl(u"https://other.example/x"_s));
}

void WebDavClientNoNetTest::check207KeepsPercentEncodedHref()
{
    // 保留百分号编码形态：不得被 QUrl 规范化为字面空格（真实服务器常态）
    const auto body = QByteArrayLiteral("<D:multistatus xmlns:D=\"DAV:\"><D:response>"
                                        "<D:href>/dav/my%20file.txt</D:href>"
                                        "</D:response></D:multistatus>");
    const auto checked
        = WebDavClient::check(makeResult(207, body, QUrl(u"http://example.org/dav/"_s)));

    QVERIFY(checked);
    QCOMPARE(checked.resources.at(0).href.toString(QUrl::FullyEncoded),
             QString::fromLatin1("http://example.org/dav/my%20file.txt"));
}

void WebDavClientNoNetTest::check207ResponseStatusLine()
{
    // PROPFIND 信封成功，但单个资源可为 404：逐资源状态进 resources
    const auto body = QByteArrayLiteral("<D:multistatus xmlns:D=\"DAV:\"><D:response>"
                                        "<D:href>/dav/gone.txt</D:href>"
                                        "<D:status>HTTP/1.1 404 Not Found</D:status>"
                                        "</D:response></D:multistatus>");
    const auto checked
        = WebDavClient::check(makeResult(207, body, QUrl(u"http://example.org/dav/"_s)));

    QVERIFY(checked);
    QCOMPARE(checked.resources.size(), 1);
    QCOMPARE(checked.resources.at(0).status, 404);
}

void WebDavClientNoNetTest::check207SkipsUnknownElementsAndInvalidProps()
{
    // 宽容解析：非 response 元素与未知属性（creationdate）跳过；
    // 非法 getcontentlength 落到 -1、非法 getlastmodified 落到无效
    const auto body = QByteArrayLiteral("<D:multistatus xmlns:D=\"DAV:\">"
                                        "<D:responsedescription>partial</D:responsedescription>"
                                        "<D:response><D:href>/lenient.txt</D:href>"
                                        "<D:propstat><D:prop>"
                                        "<D:displayname>lenient</D:displayname>"
                                        "<D:creationdate>2026-10-13T00:00:00Z</D:creationdate>"
                                        "<D:getcontentlength>not-a-number</D:getcontentlength>"
                                        "<D:getcontenttype>text/plain</D:getcontenttype>"
                                        "<D:getlastmodified>not an http date</D:getlastmodified>"
                                        "</D:prop><D:status>HTTP/1.1 200 OK</D:status></D:propstat>"
                                        "</D:response></D:multistatus>");
    const auto checked = WebDavClient::check(makeResult(207, body, QUrl(u"http://example.org/"_s)));

    QVERIFY(checked);
    QCOMPARE(checked.resources.size(), 1);
    const auto &r = checked.resources.at(0);
    QCOMPARE(r.displayName, u"lenient"_s);
    QCOMPARE(r.contentLength, -1);
    QCOMPARE(r.contentType, u"text/plain"_s);
    QVERIFY(!r.lastModified.isValid());
    QCOMPARE(r.status, 200);
}

void WebDavClientNoNetTest::check207MixedPropstatFails()
{
    // PROPPATCH 恒回 207：同一 response 混合 2xx/4xx propstat 时，
    // 已生效属性保留、失败属性丢弃，但整体判失败（failedProps）
    const auto body
        = QByteArrayLiteral("<D:multistatus xmlns:D=\"DAV:\"><D:response><D:href>/a.txt</D:href>"
                            "<D:propstat><D:prop><D:displayname>a</D:displayname></D:prop>"
                            "<D:status>HTTP/1.1 200 OK</D:status></D:propstat>"
                            "<D:propstat><D:prop><D:getetag>\"v1\"</D:getetag></D:prop>"
                            "<D:status>HTTP/1.1 403 Forbidden</D:status></D:propstat>"
                            "</D:response></D:multistatus>");
    const auto checked
        = WebDavClient::check(makeResult(207, body, QUrl(u"http://example.org/"_s), "PROPPATCH"));

    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Multistatus);
    QVERIFY(checked.message.contains(u"1 of 1"_s));
    QCOMPARE(checked.resources.size(), 1);
    QVERIFY(checked.resources.at(0).failedProps);
    QCOMPARE(checked.resources.at(0).displayName, u"a"_s);
    QVERIFY(checked.resources.at(0).etag.isEmpty()); // 403 propstat 的属性不进结果
}

void WebDavClientNoNetTest::check207MissingHrefFails()
{
    const auto body = QByteArrayLiteral("<D:multistatus xmlns:D=\"DAV:\"><D:response>"
                                        "<D:status>HTTP/1.1 404 Not Found</D:status>"
                                        "</D:response></D:multistatus>");
    const auto checked = WebDavClient::check(makeResult(207, body, QUrl(u"http://example.org/"_s)));

    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Multistatus);
    QVERIFY(checked.message.contains(u"Malformed"_s));
    QVERIFY(checked.resources.isEmpty());
}

void WebDavClientNoNetTest::check207MalformedBodyFails()
{
    const auto checked = WebDavClient::check(
        makeResult(207, QByteArrayLiteral("definitely not xml"), QUrl(u"http://example.org/"_s)));
    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Multistatus);
    QVERIFY(checked.resources.isEmpty());
}

void WebDavClientNoNetTest::check207EmptyBodyFails()
{
    const auto checked
        = WebDavClient::check(makeResult(207, QByteArray(), QUrl(u"http://example.org/"_s)));
    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Multistatus);
}

void WebDavClientNoNetTest::checkCopy207CountsFailures()
{
    const auto body = QByteArrayLiteral(
        "<D:multistatus xmlns:D=\"DAV:\">"
        "<D:response><D:href>/a.txt</D:href><D:status>HTTP/1.1 201 Created</D:status></D:response>"
        "<D:response><D:href>/b.txt</D:href>"
        "<D:status>HTTP/1.1 412 Precondition Failed</D:status></D:response>"
        "</D:multistatus>");
    const auto checked
        = WebDavClient::check(makeResult(207, body, QUrl(u"http://example.org/"_s), "COPY"));

    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Multistatus);
    QCOMPARE(checked.resources.size(), 2);
    QVERIFY(checked.message.contains(u"1 of 2"_s));
}

// ── 错误分类与措辞 ──

void WebDavClientNoNetTest::checkErrorClassification()
{
    const QUrl url;

    const struct
    {
        int status;
        DavErrorCode code;
    } cases[] = {
        {409, DavErrorCode::Conflict},
        {412, DavErrorCode::PreconditionFailed},
        {415, DavErrorCode::UnsupportedMediaType},
        {423, DavErrorCode::Locked},
        {502, DavErrorCode::BadGateway},
        {507, DavErrorCode::InsufficientStorage},
        {500, DavErrorCode::Other},
        {0, DavErrorCode::Other},
    };

    for (const auto &c : cases) {
        QCOMPARE(WebDavClient::check(makeResult(c.status, {}, url)).code, c.code);
    }
    // 207 无论载荷如何，分类恒为 Multistatus
    QCOMPARE(WebDavClient::check(makeResult(207, {}, url)).code, DavErrorCode::Multistatus);
}

void WebDavClientNoNetTest::checkVerbDisambiguatedMessages()
{
    const QUrl url;
    QVERIFY(WebDavClient::check(makeResult(409, {}, url, "MKCOL"))
                .message.contains(u"Parent collection"_s));
    QVERIFY(WebDavClient::check(makeResult(409, {}, url, "PUT")).message.contains(u"Conflict"_s));
    QVERIFY(WebDavClient::check(makeResult(405, {}, url, "MKCOL"))
                .message.contains(u"already exists"_s));
    QVERIFY(
        WebDavClient::check(makeResult(412, {}, url, "COPY")).message.contains(u"Overwrite=F"_s));
    QVERIFY(
        WebDavClient::check(makeResult(423, {}, url, "LOCK")).message.contains(u"conflicting"_s));
    QVERIFY(WebDavClient::check(makeResult(423, {}, url, "PUT")).message.contains(u"locked"_s));
}

void WebDavClientNoNetTest::checkUnmappedStatusPassesMessageThrough()
{
    auto result = makeResult(500, {}, QUrl(u"http://example.org/"_s), "GET");
    result.message = u"Error transferring ... - server replied: Internal Server Error"_s;

    const auto checked = WebDavClient::check(result);
    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Other);
    QCOMPARE(checked.message, result.message);
}

void WebDavClientNoNetTest::checkLocalErrorPassesMessage()
{
    auto result = makeResult(0, {}, QUrl(u"http://example.org/"_s));
    result.message = u"Request timed out"_s;

    const auto checked = WebDavClient::check(result);
    QVERIFY(!checked);
    QCOMPARE(checked.code, DavErrorCode::Other);
    QCOMPARE(checked.message, result.message);
}

// ── 锁令牌表 ──

void WebDavClientNoNetTest::lockStoreSemantics()
{
    const QUrl url(u"http://example.org/dav/a.txt"_s);
    QVERIFY(m_dav->lockToken(url).isEmpty());
    QVERIFY(!m_dav->hasLock(url));

    m_dav->rememberLock(url, QByteArrayLiteral("tok-1"));
    QVERIFY(m_dav->hasLock(url));
    QCOMPARE(m_dav->lockToken(url), QByteArray("tok-1"));

    m_dav->forgetLock(url);
    QVERIFY(!m_dav->hasLock(url));
    QVERIFY(m_dav->lockToken(url).isEmpty());
}

void WebDavClientNoNetTest::lockStoreKeysAreExact()
{
    // 完全编码 URL 精确匹配（尾部斜杠敏感）
    m_dav->rememberLock(QUrl(u"http://example.org/dav/a"_s), QByteArrayLiteral("tok"));
    QVERIFY(m_dav->hasLock(QUrl(u"http://example.org/dav/a"_s)));
    QVERIFY(!m_dav->hasLock(QUrl(u"http://example.org/dav/a/"_s)));
    QVERIFY(!m_dav->hasLock(QUrl(u"http://other.org/dav/a"_s)));
    m_dav->forgetLock(QUrl(u"http://example.org/dav/a"_s));
}

// ── Q_ENUM 反射 ──

void WebDavClientNoNetTest::depthEnumReflection()
{
    const auto meta = QMetaEnum::fromType<WebDavClient::Depth>();
    QCOMPARE(meta.valueToKey(int(WebDavClient::Depth::Zero)), "Zero");
    QCOMPARE(meta.valueToKey(int(WebDavClient::Depth::One)), "One");
    QCOMPARE(meta.valueToKey(int(WebDavClient::Depth::Infinity)), "Infinity");
}

QTEST_MAIN(WebDavClientNoNetTest)
#include "webdavclient_nonet_unittest.moc"
