#include "logbuffer.hpp"

#include <condition_variable>
#include <mutex>
#include <utility>

namespace AsyncLog {

class LogBuffer::LogBufferPrivate
{
public:
    LogBufferPrivate(LogBuffer *q, std::size_t highWater, std::chrono::milliseconds deadline)
        : q_ptr(q), highWaterBytes(highWater), blockDeadline(deadline)
    { items.reserve(1024); }

    LogBuffer *q_ptr;
    const std::size_t highWaterBytes;
    const std::chrono::milliseconds blockDeadline;

    std::mutex mutex;
    std::condition_variable cvData;  // 后端等数据
    std::condition_variable cvSpace; // 饱和生产者等空间
    std::vector<LogRecord> items;    // 生产 push / 消费 swap
    std::size_t bytes = 0;           // items 当前积压字节估计
    std::size_t dropped = 0;
    bool degraded = false;           // 降级期：低级别直接丢弃
    bool closed = false;
};

LogBuffer::LogBuffer(std::size_t highWaterBytes, std::chrono::milliseconds blockDeadline)
    : d_ptr(std::make_unique<LogBufferPrivate>(this, highWaterBytes, blockDeadline))
{}

LogBuffer::~LogBuffer() = default;

void LogBuffer::push(LogRecord &&record)
{
    const auto bytes = record.estimateBytes();
    auto *d = d_ptr.get();
    std::unique_lock lock(d->mutex);
    if (d->closed) {
        return;
    }

    if (d->bytes + bytes > d->highWaterBytes) {
        if (!d->degraded) {
            // 首次触顶：等一个完整的截止期给后端追平的机会，吸收瞬时尖峰
            d->degraded = true;
            d->cvSpace.wait_until(lock, std::chrono::steady_clock::now() + d->blockDeadline, [&] {
                return d->closed || d->bytes + bytes <= d->highWaterBytes;
            });
            if (d->closed) {
                return;
            }
        }
        if (d->bytes + bytes > d->highWaterBytes) {
            // 降级期仍满：错误级无条件收下（宁可内存短暂超限），低级别丢弃
            if (severity(record.type) < severity(QtWarningMsg)) {
                ++d->dropped;
                return;
            }
        }
    } else if (d->degraded && d->bytes + bytes <= d->highWaterBytes / 2) {
        d->degraded = false; // 积压回落到一半以下，退出降级期
    }

    const bool wasIdle = d->items.empty();
    d->bytes += bytes;
    d->items.push_back(std::move(record));
    if (wasIdle) {
        d->cvData.notify_one(); // 边沿触发：只在队列从空到非空时唤醒
    }
}

bool LogBuffer::popBatch(std::vector<LogRecord> &out, std::chrono::milliseconds timeout)
{
    auto *d = d_ptr.get();
    std::unique_lock lock(d->mutex);
    d->cvData.wait_for(lock, timeout, [&] { return !d->items.empty() || d->closed; });
    if (d->items.empty()) {
        return !d->closed; // close 且排空 → 后端退出；超时空等 → 继续
    }
    out.swap(d->items);    // O(1)：旧缓冲的容量留给下一批生产
    d->items.clear();      // 丢弃换入的上一批残余（调用方处理完已清空时为空操作）
    d->bytes = 0;
    d->cvSpace.notify_all();
    return true;
}

void LogBuffer::close()
{
    auto *d = d_ptr.get();
    {
        std::lock_guard lock(d->mutex);
        if (d->closed) {
            return;
        }
        d->closed = true;
    }
    d->cvData.notify_all();
    d->cvSpace.notify_all();
}

void LogBuffer::reopen()
{
    auto *d = d_ptr.get();
    std::lock_guard lock(d->mutex);
    d->closed = false;
    d->degraded = false;
    d->dropped = 0;
    d->bytes = 0;
    d->items.clear();
}

std::size_t LogBuffer::takeDropped()
{
    auto *d = d_ptr.get();
    std::lock_guard lock(d->mutex);
    return std::exchange(d->dropped, std::size_t{0});
}

} // namespace AsyncLog
