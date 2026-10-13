#pragma once

#include <chrono>
#include <cstddef>
#include <memory>
#include <vector>

#include <QString>
#include <QtGlobal>

namespace AsyncLog {

// QtMsgType 的数值顺序不等于严重性顺序（QtInfoMsg == 4 高于 QtFatalMsg == 3），
// 显式映射成连续的严重度，级别过滤必须基于该值
[[nodiscard]] constexpr int severity(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg: return 0;
    case QtInfoMsg: return 1;
    case QtWarningMsg: return 2;
    case QtCriticalMsg: return 3;
    case QtFatalMsg: return 4;
    }
    return 5;
}

// 单条日志记录。捕获端只做浅拷贝（QString 引用计数 +1），格式化推迟到后端线程
struct LogRecord
{
    qint64 timestampNs{0}; // std::chrono::system_clock 纳秒
    QtMsgType type{QtDebugMsg};
    QString message;
    quintptr threadId{0};      // 原生线程 id，后端以 {:x} 格式化
    const char *category{"default"};
    const char *file{nullptr}; // 仅 debug 构建有效，release 可能为 nullptr
    int line{0};

    // 积压量的粗略估计，仅用于饱和判断（sizeof + UTF-16 字符数据）
    [[nodiscard]] std::size_t estimateBytes() const
    { return sizeof(LogRecord) + std::size_t(message.size()) * 2; }
};

// MPSC 日志队列：多个生产线程 push，单个后端线程 popBatch 整批取走。
//
// 分层饱和策略（积压超过 highWaterBytes 时）：
//   1. 首次触顶：生产者等待 blockDeadline 给后端追平的机会，吸收瞬时尖峰
//   2. 等待超时进入降级期：WARNING 及以上无条件收下（错误级绝不丢），
//      DEBUG/INFO 丢弃并计数，生产者不再阻塞
//   3. 后端恢复后通过 takeDropped() 取走计数并补报
class LogBuffer
{
public:
    explicit LogBuffer(std::size_t highWaterBytes = 32 * 1024 * 1024,
                       std::chrono::milliseconds blockDeadline = std::chrono::milliseconds{1000});
    ~LogBuffer();

    LogBuffer(const LogBuffer &) = delete;
    LogBuffer &operator=(const LogBuffer &) = delete;

    // 生产端（多线程）。close 后静默丢弃
    void push(LogRecord &&record);

    // 消费端（单线程）。等待数据或 close，最多 timeout。
    // 返回 false 表示已 close 且排空——后端应当退出
    [[nodiscard]] bool popBatch(std::vector<LogRecord> &out, std::chrono::milliseconds timeout);

    // 停止接受新消息并唤醒后端做最终排空（排空由后续 popBatch 完成）
    void close();

    // 重新开放（start/shutdown 循环复用；要求已排空）
    void reopen();

    // 取走溢出丢弃计数（后端补报用）
    [[nodiscard]] std::size_t takeDropped();

private:
    class LogBufferPrivate;
    std::unique_ptr<LogBufferPrivate> d_ptr;
};

} // namespace AsyncLog
