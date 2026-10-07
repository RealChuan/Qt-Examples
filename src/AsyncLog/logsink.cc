#include "logsink.hpp"

#include <QString>

#include <cstdio>
#include <utility>

namespace AsyncLog {

class ConsoleSink::ConsoleSinkPrivate
{
public:
    ConsoleSinkPrivate(ConsoleSink *q,
                       std::size_t truncate,
                       std::FILE *outStream,
                       std::FILE *errStream)
        : q_ptr(q), truncateBytes(truncate), out(outStream), err(errStream)
    {}

    ConsoleSink *q_ptr;
    const std::size_t truncateBytes;
    std::FILE *out;
    std::FILE *err;
};

ConsoleSink::ConsoleSink(std::size_t truncateBytes, std::FILE *out, std::FILE *err)
    : d_ptr(std::make_unique<ConsoleSinkPrivate>(this, truncateBytes, out, err))
{}

ConsoleSink::~ConsoleSink() = default;

void ConsoleSink::write(std::span<const char> bytes, bool isError)
{
    auto *d = d_ptr.get();
    auto *stream = isError ? d->err : d->out;
    auto size = bytes.size();
    const bool truncated = d->truncateBytes > 0 && size > d->truncateBytes;
    if (truncated) {
        size = d->truncateBytes;
    }

#ifdef Q_OS_WIN
    const QString q = QString::fromUtf8(bytes.data(), static_cast<int>(size));
    const QByteArray local = q.toLocal8Bit();
    std::fwrite(local.constData(), 1, static_cast<std::size_t>(local.size()), stream);
    if (truncated) {
        std::fputc('\n', stream);
    }
    return;
#endif

    std::fwrite(bytes.data(), 1, size, stream);
    if (truncated) {
        std::fputc('\n', stream);
    }
}

void ConsoleSink::flush()
{
    auto *d = d_ptr.get();
    std::fflush(d->out);
    std::fflush(d->err);
}

} // namespace AsyncLog
