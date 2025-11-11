#include "autostartmanager.hpp"

#include <QApplication>
#include <QDir>
#include <QSettings>

namespace Utils {

// 注册表路径定义
static const QString REG_RUN_PATH
    = "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

bool isAutoRunStart()
{
    QString appName = qApp->applicationName();
    QString currentPath = QSettings(REG_RUN_PATH, QSettings::NativeFormat).value(appName).toString();
    QString expectedPath = QString("\"%1\"").arg(
        QDir::toNativeSeparators(qApp->applicationFilePath()));

    return (currentPath == expectedPath);
}

void setAutoRunStart(bool run)
{
    QString appName = qApp->applicationName();
    QSettings settings(REG_RUN_PATH, QSettings::NativeFormat);

    if (run) {
        QString appPath = QString("\"%1\"").arg(
            QDir::toNativeSeparators(qApp->applicationFilePath()));
        settings.setValue(appName, appPath);
        qInfo() << "Auto-run enabled for:" << appPath;
    } else {
        settings.remove(appName);
        qInfo() << "Auto-run disabled for:" << appName;
    }
}

} // namespace Utils
