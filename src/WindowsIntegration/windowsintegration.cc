#include "windowsintegration.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

using namespace Qt::StringLiterals;

namespace Utils {

namespace {

// 注册表路径常量
const QString REG_COMPUTER_NAMESPACE_PATH
    = u"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace"_s;
const QString REG_CLASSES_CLSID_PATH = u"HKEY_CURRENT_USER\\SOFTWARE\\Classes\\CLSID"_s;

} // namespace

[[nodiscard]] std::expected<bool, QString> isRegisteredInMyComputer(const QString &guid)
{
    // 检查 CLSID 注册表项是否存在
    const QSettings clsidSettings(REG_CLASSES_CLSID_PATH, QSettings::NativeFormat);
    if (clsidSettings.status() != QSettings::NoError) {
        return std::unexpected(u"Failed to read CLSID registry"_s);
    }
    if (!clsidSettings.childGroups().contains(guid)) {
        return false;
    }

    // 检查命名空间注册表项是否存在
    const QSettings namespaceSettings(REG_COMPUTER_NAMESPACE_PATH, QSettings::NativeFormat);
    if (namespaceSettings.status() != QSettings::NoError) {
        return std::unexpected(u"Failed to read namespace registry"_s);
    }

    return namespaceSettings.childGroups().contains(guid);
}

[[nodiscard]] std::expected<void, QString> setRegisteredInMyComputer(bool registerApp,
                                                                     const QString &guid)
{
    const QString appName = QCoreApplication::applicationName();
    const QString appPath
        = u"\"%1\""_s.arg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));

    if (registerApp) {
        // 注册 CLSID 信息
        QSettings clsidSettings(u"%1\\%2"_s.arg(REG_CLASSES_CLSID_PATH, guid),
                                QSettings::NativeFormat);
        clsidSettings.setValue(u"."_s, appName);       // 显示名称
        clsidSettings.setValue(u"InfoTip"_s, appName); // 悬停提示
        if (clsidSettings.status() != QSettings::NoError) {
            return std::unexpected(u"Failed to write CLSID registry"_s);
        }

        // 注册图标
        QSettings iconSettings(u"%1\\%2\\DefaultIcon"_s.arg(REG_CLASSES_CLSID_PATH, guid),
                               QSettings::NativeFormat);
        iconSettings.setValue(u"."_s, appPath);
        if (iconSettings.status() != QSettings::NoError) {
            return std::unexpected(u"Failed to write icon registry"_s);
        }

        // 注册打开命令
        QSettings commandSettings(
            u"%1\\%2\\shell\\Open\\Command"_s.arg(REG_CLASSES_CLSID_PATH, guid),
            QSettings::NativeFormat);
        commandSettings.setValue(u"."_s, u"%1 \"%1\""_s.arg(appPath));
        if (commandSettings.status() != QSettings::NoError) {
            return std::unexpected(u"Failed to write command registry"_s);
        }

        // 在"此电脑"命名空间中注册
        QSettings namespaceSettings(u"%1\\%2"_s.arg(REG_COMPUTER_NAMESPACE_PATH, guid),
                                    QSettings::NativeFormat);
        namespaceSettings.setValue(u"."_s, appName);
        if (namespaceSettings.status() != QSettings::NoError) {
            return std::unexpected(u"Failed to write namespace registry"_s);
        }

        qInfo() << "My Computer integration enabled for:" << appName;
    } else {
        // 取消注册：删除命名空间注册
        QSettings namespaceSettings(REG_COMPUTER_NAMESPACE_PATH, QSettings::NativeFormat);
        namespaceSettings.remove(guid);
        if (namespaceSettings.status() != QSettings::NoError) {
            return std::unexpected(u"Failed to remove namespace registry"_s);
        }

        // 删除 CLSID 注册
        QSettings clsidSettings(REG_CLASSES_CLSID_PATH, QSettings::NativeFormat);
        clsidSettings.remove(guid);
        if (clsidSettings.status() != QSettings::NoError) {
            return std::unexpected(u"Failed to remove CLSID registry"_s);
        }

        qInfo() << "My Computer integration disabled for:" << appName;
    }

    return {};
}

} // namespace Utils
