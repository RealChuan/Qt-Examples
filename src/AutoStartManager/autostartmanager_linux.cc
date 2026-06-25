#include "autostartmanager.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>

using namespace Qt::StringLiterals;

namespace Utils {

// 获取 autostart 目录路径（遵循 XDG）
static QString autostartDirPath()
{
    return u"%1/autostart"_s.arg(
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
}

static QString desktopFilePath()
{ return autostartDirPath() + u"/%1.desktop"_s.arg(QCoreApplication::applicationName()); }

[[nodiscard]] std::expected<bool, QString> isAutoRunStart()
{ return QFile::exists(desktopFilePath()); }

[[nodiscard]] std::expected<void, QString> setAutoRunStart(bool run)
{
    const QString filePath = desktopFilePath();

    if (run) {
        // 确保目录存在
        const QDir autostartDir(autostartDirPath());
        if (!autostartDir.exists() && !autostartDir.mkpath(u"."_s)) {
            return std::unexpected(u"Failed to create autostart directory"_s);
        }

        const QString desktopContent = uR"([Desktop Entry]
Type=Application
Name=%1
Exec=%2
Hidden=false
NoDisplay=false
X-GNOME-Autostart-enabled=true
)"_s.arg(QCoreApplication::applicationName(), QCoreApplication::applicationFilePath());

        // 原子写入：QSaveFile 先写临时文件再原子替换
        QSaveFile desktopFile(filePath);
        if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return std::unexpected(
                u"Failed to create desktop file: %1"_s.arg(desktopFile.errorString()));
        }
        if (desktopFile.write(desktopContent.toUtf8()) == -1) {
            return std::unexpected(
                u"Failed to write desktop file: %1"_s.arg(desktopFile.errorString()));
        }
        if (!desktopFile.commit()) {
            return std::unexpected(
                u"Failed to commit desktop file: %1"_s.arg(desktopFile.errorString()));
        }
    } else {
        if (QFile::exists(filePath) && !QFile::remove(filePath)) {
            return std::unexpected(u"Failed to remove desktop file"_s);
        }
    }

    qInfo() << "Auto-run" << (run ? "enabled" : "disabled");
    return {};
}

} // namespace Utils
