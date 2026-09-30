#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include <memory>

class QMessageLogContext;

namespace AsyncLog {

// 一次性配置。除 level（运行期可经 Logger::setLevel 原子调整）外，
// 其余项在 Logger::start 后固定
struct Config
{
    QString logPath;                          // 空 = 不写文件
    bool console = true;                      // 控制台输出
    bool file = false;                        // 文件输出（需要 logPath）
    QtMsgType level = QtInfoMsg;              // 最低输出级别
    qint64 maxFileSize = 512LL * 1024 * 1024; // 单文件滚动阈值
    qint64 keepDays = 7;                      // 过期清理天数，0 = 不清理
    qsizetype consoleTruncate = 0;            // 控制台单行截断字节数，0 = 不截断
};

// 异步日志门面：接管 Qt 全局日志（qDebug/qInfo/qWarning/...），批量落盘。
//
// 用法：
//     AsyncLog::Logger::instance()->start({.logPath = "logs", .file = true});
//     qInfo() << "hello";
//     AsyncLog::Logger::instance()->shutdown();   // main 返回前调用，保证日志落盘
class Logger : public QObject
{
    Q_OBJECT

public:
    // 单例。首次调用所在线程即 Logger 的宿主线程（先在 main 里 start 即可）
    static Logger *instance();

    // 安装消息处理器并启动后端线程；幂等（运行中重复调用被忽略）
    void start(const Config &config);

    // 卸载处理器 → 排空队列 → join。返回时所有已入队日志都已写入；
    // 幂等。忘记调用时由析构兜底（此时进程即将退出）
    void shutdown();

    void setLevel(QtMsgType level); // 原子生效
    [[nodiscard]] auto level() const -> QtMsgType;
    [[nodiscard]] auto isRunning() const -> bool;

    // 删除日志目录下所有匹配自身命名模式的历史日志（含其他进程实例留下的），
    // 正在写入的当前文件除外；返回删除的文件数。
    // 需在运行期间调用（未运行或未启用文件输出时返回 0）
    [[nodiscard]] auto purgeLogFiles() -> int;

signals:
    // 每消费完一批发出（在后端线程 emit，跨线程接收走默认的 AutoConnection；
    // 无人连接时跳过批次构建）。UI 类消费者可用它做实时日志视图
    void batchReady(const QStringList &lines);

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger() override; // 兜底 shutdown

    class LoggerPrivate;
    std::unique_ptr<LoggerPrivate> d_ptr;

    friend void
    messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);
};

// qInstallMessageHandler 的目标函数，单测可直接调用以覆盖 fatal 路径
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);

} // namespace AsyncLog
