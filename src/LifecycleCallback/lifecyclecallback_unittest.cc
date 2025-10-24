#include "lifecyclecallback.hpp"

#include <QObject>
#include <QtTest>

#include <functional>
#include <memory>

// 测试用的辅助类
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
        lastMessage = QString("%1/%2").arg(downloaded).arg(total);
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
        lastMessage = QString("%1/%2").arg(downloaded).arg(total);
    }
};

// 自由函数
void freeFunctionMessage(const QString &message)
{
    static int callCount = 0;
    callCount++;
    Q_UNUSED(message)
}

void freeFunctionJson(const QJsonObject &json)
{
    static int callCount = 0;
    callCount++;
    Q_UNUSED(json)
}

class TestLifecycleCallback : public QObject
{
    Q_OBJECT

private slots:
    // 测试默认构造
    void testDefaultConstruction()
    {
        LifecycleCallback<> callback;
        QVERIFY(!callback);
        callback(); // 应该不会崩溃
    }

    // 测试自由函数回调
    void testFreeFunction()
    {
        LifecycleCallback<const QString &> callback(freeFunctionMessage);
        QVERIFY(callback);
        callback("test message");

        LifecycleCallback<const QJsonObject &> jsonCallback(freeFunctionJson);
        QVERIFY(jsonCallback);

        QJsonObject testJson;
        testJson["key"] = "value";
        jsonCallback(testJson);
    }

    // 测试 Lambda 回调
    void testLambda()
    {
        int callCount = 0;
        QString lastMessage;

        LifecycleCallback<const QString &> callback([&](const QString &msg) {
            callCount++;
            lastMessage = msg;
        });

        QVERIFY(callback);

        callback("lambda test");
        QCOMPARE(callCount, 1);
        QCOMPARE(lastMessage, "lambda test");

        callback("another test");
        QCOMPARE(callCount, 2);
        QCOMPARE(lastMessage, "another test");
    }

    // 测试 QObject 成员函数回调
    void testQObjectMember_alive()
    {
        auto *testObj = new TestQObject;
        LifecycleCallback<const QString &> callback
            = makeLifecycleCallback(testObj, &TestQObject::handleMessage);

        QVERIFY(callback);

        callback("qobject test");
        QCOMPARE(testObj->callCount, 1);
        QCOMPARE(testObj->lastMessage, "qobject test");

        delete testObj;
    }

    // 测试 QObject 成员函数回调（对象销毁）
    void testQObjectMember_destroyed()
    {
        auto *testObj = new TestQObject;
        LifecycleCallback<const QString &> callback
            = makeLifecycleCallback(testObj, &TestQObject::handleMessage);

        QVERIFY(callback);

        delete testObj;

        QVERIFY(!callback);
        callback("should not be handled");
    }

    // 测试 std::shared_ptr 成员函数回调
    void testSharedPtrMember_alive()
    {
        auto testObj = std::make_shared<TestSharedObject>();
        LifecycleCallback<const QString &> callback
            = makeLifecycleCallback(testObj, &TestSharedObject::handleMessage);

        QVERIFY(callback);

        callback("shared_ptr test");
        QCOMPARE(testObj->callCount, 1);
        QCOMPARE(testObj->lastMessage, "shared_ptr test");
    }

    // 测试 std::shared_ptr 成员函数回调（对象销毁）
    void testSharedPtrMember_destroyed()
    {
        auto testObj = std::make_shared<TestSharedObject>();
        LifecycleCallback<const QString &> callback
            = makeLifecycleCallback(testObj, &TestSharedObject::handleMessage);

        QVERIFY(callback);

        testObj.reset();

        QVERIFY(!callback);
        callback("should not be handled");
    }

    // 测试多种参数类型
    void testMultipleParameters()
    {
        // 测试无参数
        int callCountNoArgs = 0;
        LifecycleCallback<> noArgsCallback([&]() { callCountNoArgs++; });
        noArgsCallback();
        QCOMPARE(callCountNoArgs, 1);

        // 测试单个参数
        int callCountOneArg = 0;
        QString lastArg;
        LifecycleCallback<const QString &> oneArgCallback([&](const QString &arg) {
            callCountOneArg++;
            lastArg = arg;
        });
        oneArgCallback("test");
        QCOMPARE(callCountOneArg, 1);
        QCOMPARE(lastArg, "test");

        // 测试多个参数
        int callCountMultiArgs = 0;
        qint64 lastDownloaded = 0, lastTotal = 0;
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

    // 测试移动语义
    void testMoveSemantics()
    {
        int callCount1 = 0;
        LifecycleCallback<> callback1([&]() { callCount1++; });

        LifecycleCallback<> callback2(std::move(callback1));
        QVERIFY(!callback1);
        QVERIFY(callback2);

        callback2();
        QCOMPARE(callCount1, 1);
    }

    // 测试工厂函数
    void testFactoryFunctions()
    {
        auto *qobject = new TestQObject;
        auto qobjectCallback = makeLifecycleCallback(qobject, &TestQObject::handleMessage);
        qobjectCallback("factory test");
        QCOMPARE(qobject->callCount, 1);
        delete qobject;

        auto sharedObj = std::make_shared<TestSharedObject>();
        auto sharedCallback = makeLifecycleCallback(sharedObj, &TestSharedObject::handleMessage);
        sharedCallback("shared factory test");
        QCOMPARE(sharedObj->callCount, 1);
    }

    // 测试空 std::function
    void testEmptyStdFunction()
    {
        std::function<void()> emptyFunc;
        LifecycleCallback<> callback(emptyFunc);
        QVERIFY(!callback);

        std::function<void()> emptyFunc2 = nullptr;
        LifecycleCallback<> callback2(emptyFunc2);
        QVERIFY(!callback2);
    }

    // 测试生命周期安全
    void testLifecycleSafety()
    {
        // QObject 生命周期安全
        auto *qobject = new TestQObject;
        LifecycleCallback<const QString &> callback1
            = makeLifecycleCallback(qobject, &TestQObject::handleMessage);
        delete qobject;
        callback1("after deletion");
        QVERIFY(!callback1);

        // shared_ptr 生命周期安全
        auto sharedObj = std::make_shared<TestSharedObject>();
        LifecycleCallback<const QString &> callback2
            = makeLifecycleCallback(sharedObj, &TestSharedObject::handleMessage);
        sharedObj.reset();
        for (int i = 0; i < 10; ++i) {
            callback2("after reset");
        }
        QVERIFY(!callback2);
    }
};

// 注册测试
QTEST_APPLESS_MAIN(TestLifecycleCallback)

#include "lifecyclecallback_unittest.moc"
