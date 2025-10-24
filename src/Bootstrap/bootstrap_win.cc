#include "bootstrap.hpp"

#include <QApplication>
#include <QDir>
#include <QSettings>

static const QString REG_RUN_PATH
    = "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
#define APP_NAME qApp->applicationName()
#define App_FILE_PATH QString("\"%1\"").arg(QDir::toNativeSeparators(qApp->applicationFilePath()))

namespace Bootstrap {

bool isAutoRunStart()
{
    QScopedPointer<QSettings> settings(new QSettings(REG_RUN_PATH, QSettings::NativeFormat));
    const QString path = settings->value(APP_NAME).toString();

    return (path == App_FILE_PATH);
}

void setAutoRunStart(bool run)
{
    QSettings reg(REG_RUN_PATH, QSettings::NativeFormat);
    if (run) {
        reg.setValue(APP_NAME, App_FILE_PATH);
    } else {
        reg.remove(APP_NAME);
    }
}

} // namespace Bootstrap
