#pragma once

#include <cstddef>
#include <cstdio>
#include <memory>
#include <span>

namespace AsyncLog {

// 输出目的地抽象。write 除后端线程外还可能被 fatal 同步路径并发调用，
// 实现自行保证线程安全
class ISink
{
public:
    virtual ~ISink() = default;

    // bytes 为一整行 UTF-8（含换行符）；isError 表示该行是否为 WARNING 及以上级别
    virtual void write(std::span<const char> bytes, bool isError) = 0;
    virtual void flush() = 0;
};

// 控制台输出：常规级别走 stdout，错误级别走 stderr；stdio 自带缓冲，
// flush 时一次落盘。构造参数允许注入 FILE*（单测捕获输出用）
class ConsoleSink final : public ISink
{
public:
    // truncateBytes > 0 时每行截断到该字节数（按字节截断可能切断 UTF-8 序列，
    // 控制台展示可接受；文件 sink 始终写全量）
    explicit ConsoleSink(std::size_t truncateBytes = 0,
                         std::FILE *out = stdout,
                         std::FILE *err = stderr);
    ~ConsoleSink() override;

    void write(std::span<const char> bytes, bool isError) override;
    void flush() override;

private:
    class ConsoleSinkPrivate;
    std::unique_ptr<ConsoleSinkPrivate> d_ptr;
};

} // namespace AsyncLog
