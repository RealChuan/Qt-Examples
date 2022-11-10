#include "functions.hpp"

#include <QThread>

QString Functions::getCurrentThreadIDStr()
{
    return QString("Current Thread ID: %1.")
        .arg(reinterpret_cast<qint64>(QThread::currentThreadId()));
}
