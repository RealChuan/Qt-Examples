#include "autostartmanager.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

using namespace Qt::StringLiterals;

namespace Utils {

// 注册表路径定义
static const QString REG_RUN_PATH
    = u"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"_s;

[[nodiscard]] std::expected<bool, QString> isAutoRunStart()
{
    const QString appName = QCoreApplication::applicationName();
    const QSettings settings(REG_RUN_PATH, QSettings::NativeFormat);
    const QString currentPath = settings.value(appName).toString();

    if (currentPath.isEmpty()) {
        return false;
    }

    // Windows 文件系统大小写不敏感：规范化后按大小写不敏感比较
    const QString expectedPath
        = u"\"%1\""_s.arg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    return QString::compare(currentPath, expectedPath, Qt::CaseInsensitive) == 0;
}

[[nodiscard]] std::expected<void, QString> setAutoRunStart(bool run)
{
    const QString appName = QCoreApplication::applicationName();
    QSettings settings(REG_RUN_PATH, QSettings::NativeFormat);

    if (run) {
        const QString appPath
            = u"\"%1\""_s.arg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
        settings.setValue(appName, appPath);
    } else {
        settings.remove(appName);
    }

    if (settings.status() != QSettings::NoError) {
        return std::unexpected(u"Failed to access Windows registry"_s);
    }

    qInfo() << "Auto-run" << (run ? "enabled" : "disabled") << "for:" << appName;
    return {};
}

} // namespace Utils
