#include "autostartmanager.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>

namespace Utils {

bool isAutoRunStart()
{
    QString appName = qApp->applicationName();
    QString desktopFilePath = QDir::homePath()
                              + QString("/.config/autostart/%1.desktop").arg(appName);

    return QFile::exists(desktopFilePath);
}

void setAutoRunStart(bool run)
{
    QString appName = qApp->applicationName();
    QDir autostartDir(QDir::homePath() + "/.config/autostart");

    if (!autostartDir.exists() && !autostartDir.mkpath(".")) {
        qWarning() << "Failed to create autostart directory";
        return;
    }

    QString desktopFilePath = autostartDir.filePath(appName + ".desktop");

    if (run) {
        // 创建桌面文件
        QString desktopContent = QString(R"(
[Desktop Entry]
Type=Application
Name=%1
Exec=%2
Hidden=false
NoDisplay=false
X-GNOME-Autostart-enabled=true
)")
                                     .arg(appName, QCoreApplication::applicationFilePath());

        QFile desktopFile(desktopFilePath);
        if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Failed to create desktop file:" << desktopFile.errorString();
            return;
        }

        if (desktopFile.write(desktopContent.toUtf8()) == -1) {
            qWarning() << "Failed to write desktop file:" << desktopFile.errorString();
            desktopFile.close();
            return;
        }
        desktopFile.close();

        qInfo() << "Auto-run enabled using desktop file";
    } else {
        // 删除桌面文件
        if (QFile::exists(desktopFilePath)) {
            if (!QFile::remove(desktopFilePath)) {
                qWarning() << "Failed to remove desktop file";
            } else {
                qInfo() << "Auto-run disabled";
            }
        }
    }
}

} // namespace Utils
