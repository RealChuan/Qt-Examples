#include "fileinfo.hpp"

auto bytesToString(qint64 bytes) -> QString
{
    static const double KB = 1024 * 1.0;
    static const double MB = KB * 1024;
    static const double GB = MB * 1024;
    static const double TB = GB * 1024;

    if (bytes / TB >= 1) {
        return QString("%1TB").arg(QString::number(bytes / TB, 'f', 1));
    } else if (bytes / GB >= 1) {
        return QString("%1GB").arg(QString::number(bytes / GB, 'f', 1));
    } else if (bytes / MB >= 1) {
        return QString("%1MB").arg(QString::number(bytes / MB, 'f', 1));
    } else if (bytes / KB >= 1) {
        return QString("%1KB").arg(qint64(bytes / KB));
    }
    return QString("%1B").arg(bytes);
}
