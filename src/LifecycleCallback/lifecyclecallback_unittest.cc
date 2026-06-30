#include "lifecyclecallback_qt.hpp"

#include <QJsonObject>
#include <QObject>
#include <QTest>

#include <functional>
#include <memory>

using namespace Qt::StringLiterals;

namespace {

int freeFunctionCallCount = 0;

void freeFunctionMessage(const QString &message)
{
    ++freeFunctionCallCount;
    Q_UNUSED(message)
}

void freeFunctionJson(const QJsonObject &json)
{ Q_UNUSED(json) }

} // namespace

class TestQObject : public QObject
{
    Q_OBJECT
public:
    int callCount = 0;
    QString lastMessage;
    QJsonObject lastJson;

public slots:

    void handleMessage(const QString &message)
    {
        callCount++;
        lastMessage = message;
    }

    void handleJson(const QJsonObject &json)
    {
        callCount++;
        lastJson = json;
    }

    void handleProgress(qint64 downloaded, qint64 total)
    {
        callCount++;
        lastMessage = QString(u"%1/%2"_s).arg(downloaded).arg(total);
    }
};

class TestSharedObject
{
public:
    int callCount = 0;
    QString lastMessage;
    QJsonObject lastJson;

    void handleMessage(const QString &message)
    {
        callCount++;
        lastMessage = message;
    }

    void handleJson(const QJsonObject &json)
    {
        callCount++;
        lastJson = json;
    }

    void handleProgress(qint64 downloaded, qint64 total)
    {
        callCount++;
        lastMessage = QString(u"%1/%2"_s).arg(downloaded).arg(total);
    }
};

class TestLifecycleCallback : public QObject
{
    Q_OBJECT

private slots:

    void testDefaultConstruction()
    {
        LifecycleCallback<> callback;
        QVERIFY(!callback);
        callback(); // 不会崩溃
    }

    void testFreeFunction()
    {
        freeFunctionCallCount = 0;

        LifecycleCallback<const QString &> callback(freeFunctionMessage);
        QVERIFY(callback);
        callback(u"test message"_s);
        QCOMPARE(freeFunctionCallCount, 1);

        LifecycleCallback<const QJsonObject &> jsonCallback(freeFunctionJson);
        QVERIFY(jsonCallback);

        QJsonObject testJson;
        testJson[u"key"_s] = u"value"_s;
        jsonCallback(testJson);
    }

    void testLambda()
    {
        int callCount = 0;
        QString lastMessage;

        LifecycleCallback<const QString &> callback([&](const QString &msg) {
            callCount++;
            lastMessage = msg;
        });

        QVERIFY(callback);

        callback(u"lambda test"_s);
        QCOMPARE(callCount, 1);
        QCOMPARE(lastMessage, u"lambda test"_s);

        callback(u"another test"_s);
        QCOMPARE(callCount, 2);
        QCOMPARE(lastMessage, u"another test"_s);
    }

    void testQObjectMember_alive()
    {
        auto testObj = std::make_unique<TestQObject>();
        auto *rawObj = testObj.get();

        LifecycleCallback<const QString &> callback
            = makeLifecycleCallback(rawObj, &TestQObject::handleMessage);

        QVERIFY(callback);

        callback(u"qobject test"_s);
        QCOMPARE(rawObj->callCount, 1);
        QCOMPARE(rawObj->lastMessage, u"qobject test"_s);
    }

    void testQObjectMember_destroyed()
    {
        auto testObj = std::make_unique<TestQObject>();
        auto *rawObj = testObj.get();

        LifecycleCallback<const QString &> callback
            = makeLifecycleCallback(rawObj, &TestQObject::handleMessage);

        QVERIFY(callback);

        testObj.reset();

        QVERIFY(!callback);
        callback(u"should not be handled"_s);
    }

    void testSharedPtrMember_alive()
    {
        auto testObj = std::make_shared<TestSharedObject>();

        LifecycleCallback<const QString &> callback
            = makeLifecycleCallback(testObj, &TestSharedObject::handleMessage);

        QVERIFY(callback);

        callback(u"shared_ptr test"_s);
        QCOMPARE(testObj->callCount, 1);
        QCOMPARE(testObj->lastMessage, u"shared_ptr test"_s);
    }

    void testSharedPtrMember_destroyed()
    {
        auto testObj = std::make_shared<TestSharedObject>();

        LifecycleCallback<const QString &> callback
            = makeLifecycleCallback(testObj, &TestSharedObject::handleMessage);

        QVERIFY(callback);

        testObj.reset();

        QVERIFY(!callback);
        callback(u"should not be handled"_s);
    }

    void testMultipleParameters()
    {
        int callCountNoArgs = 0;
        LifecycleCallback<> noArgsCallback([&]() { callCountNoArgs++; });
        noArgsCallback();
        QCOMPARE(callCountNoArgs, 1);

        int callCountOneArg = 0;
        QString lastArg;
        LifecycleCallback<const QString &> oneArgCallback([&](const QString &arg) {
            callCountOneArg++;
            lastArg = arg;
        });
        oneArgCallback(u"test"_s);
        QCOMPARE(callCountOneArg, 1);
        QCOMPARE(lastArg, u"test"_s);

        int callCountMultiArgs = 0;
        qint64 lastDownloaded = 0;
        qint64 lastTotal = 0;
        LifecycleCallback<qint64, qint64> multiArgsCallback([&](qint64 downloaded, qint64 total) {
            callCountMultiArgs++;
            lastDownloaded = downloaded;
            lastTotal = total;
        });
        multiArgsCallback(100, 1000);
        QCOMPARE(callCountMultiArgs, 1);
        QCOMPARE(lastDownloaded, 100);
        QCOMPARE(lastTotal, 1000);
    }

    void testMoveSemantics()
    {
        int callCount = 0;
        LifecycleCallback<> callback1([&]() { callCount++; });

        LifecycleCallback<> callback2(std::move(callback1));
        QVERIFY(!callback1);
        QVERIFY(callback2);

        callback2();
        QCOMPARE(callCount, 1);
    }

    void testPassLvalueToValueArgs()
    {
        // 值类型 Args：调用者直接传左值即可，无需在调用点书写 std::move
        QString captured;
        LifecycleCallback<QString> callback([&captured](QString s) { captured = std::move(s); });

        QString arg = u"value-arg"_s;
        callback(arg);                 // 左值直接传入
        QCOMPARE(captured, u"value-arg"_s);
        QCOMPARE(arg, u"value-arg"_s); // 原变量保持有效（被拷贝而非移动）

        qint64 a = 100;
        qint64 b = 1000;
        qint64 lastA = 0;
        qint64 lastB = 0;
        LifecycleCallback<qint64, qint64> multi([&](qint64 x, qint64 y) {
            lastA = x;
            lastB = y;
        });
        multi(a, b); // 左值 qint64 直接传入
        QCOMPARE(lastA, 100);
        QCOMPARE(lastB, 1000);
        QCOMPARE(a, 100);
        QCOMPARE(b, 1000);
    }

    void testCopySemantics()
    {
        int callCount = 0;
        LifecycleCallback<> callback1([&]() { callCount++; });

        LifecycleCallback<> callback2(callback1);
        QVERIFY(callback1);
        QVERIFY(callback2);

        callback1();
        callback2();
        QCOMPARE(callCount, 2);
    }

    void testFactoryFunctions()
    {
        auto qobject = std::make_unique<TestQObject>();
        auto qobjectCallback = makeLifecycleCallback(qobject.get(), &TestQObject::handleMessage);
        qobjectCallback(u"factory test"_s);
        QCOMPARE(qobject->callCount, 1);

        auto sharedObj = std::make_shared<TestSharedObject>();
        auto sharedCallback = makeLifecycleCallback(sharedObj, &TestSharedObject::handleMessage);
        sharedCallback(u"shared factory test"_s);
        QCOMPARE(sharedObj->callCount, 1);
    }

    void testEmptyStdFunction()
    {
        std::function<void()> emptyFunc;
        LifecycleCallback<> callback(emptyFunc);
        QVERIFY(!callback);

        std::function<void()> emptyFunc2 = nullptr;
        LifecycleCallback<> callback2(emptyFunc2);
        QVERIFY(!callback2);
    }

    void testNullFunctionPointer()
    {
        void (*nullFunc)(const QString &) = nullptr;
        LifecycleCallback<const QString &> callback(nullFunc);
        QVERIFY(!callback);
        callback(u"should not be handled"_s);
    }

    void testNullQObjectPointer()
    {
        TestQObject *nullObj = nullptr;
        auto callback = makeLifecycleCallback(nullObj, &TestQObject::handleMessage);
        QVERIFY(!callback);
        callback(u"should not be handled"_s);
    }

    void testTrackedCallback()
    {
        bool alive = true;
        int callCount = 0;

        LifecycleCallback<> callback([&callCount] { ++callCount; }, [&alive] { return alive; });

        QVERIFY(callback);
        callback();
        QCOMPARE(callCount, 1);

        alive = false;
        QVERIFY(!callback);
        callback(); // 验证器返回 false，调用器不应执行
        QCOMPARE(callCount, 1);
    }

    void testLifecycleSafety()
    {
        auto qobject = std::make_unique<TestQObject>();
        LifecycleCallback<const QString &> callback1
            = makeLifecycleCallback(qobject.get(), &TestQObject::handleMessage);
        qobject.reset();
        callback1(u"after deletion"_s);
        QVERIFY(!callback1);

        auto sharedObj = std::make_shared<TestSharedObject>();
        LifecycleCallback<const QString &> callback2
            = makeLifecycleCallback(sharedObj, &TestSharedObject::handleMessage);
        sharedObj.reset();
        for (int i = 0; i < 10; ++i) {
            callback2(u"after reset"_s);
        }
        QVERIFY(!callback2);
    }
};

QTEST_APPLESS_MAIN(TestLifecycleCallback)

#include "lifecyclecallback_unittest.moc"
