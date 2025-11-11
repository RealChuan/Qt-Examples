#include "windowsintegration.hpp"

#include <QApplication>
#include <QDir>
#include <QSettings>

namespace Utils {

namespace {
const QString REG_COMPUTER_NAMESPACE_PATH
    = "HKEY_CURRENT_"
      "USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace";
const QString REG_CLASSES_CLSID_PATH = "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\CLSID";
} // namespace

bool isRegisteredInMyComputer(const QString &guid)
{
    // 检查CLSID注册表项是否存在
    QSettings clsidSettings(REG_CLASSES_CLSID_PATH, QSettings::NativeFormat);
    if (!clsidSettings.childGroups().contains(guid)) {
        return false;
    }

    // 检查命名空间注册表项是否存在
    QSettings namespaceSettings(REG_COMPUTER_NAMESPACE_PATH, QSettings::NativeFormat);
    return namespaceSettings.childGroups().contains(guid);
}

bool setRegisteredInMyComputer(bool registerApp, const QString &guid)
{
    const QString appName = QApplication::applicationName();
    const QString appPath = QDir::toNativeSeparators(QApplication::applicationFilePath());
    const QString quotedAppPath = QString("\"%1\"").arg(appPath);

    if (registerApp) {
        // 注册 CLSID 信息
        QSettings clsidSettings(QString("%1\\%2").arg(REG_CLASSES_CLSID_PATH, guid),
                                QSettings::NativeFormat);
        clsidSettings.setValue(".", appName);       // 显示名称
        clsidSettings.setValue("InfoTip", appName); // 悬停提示

        // 注册图标
        QSettings iconSettings(QString("%1\\%2\\DefaultIcon").arg(REG_CLASSES_CLSID_PATH, guid),
                               QSettings::NativeFormat);
        iconSettings.setValue(".", quotedAppPath); // 应用程序图标

        // 注册打开命令
        QSettings commandSettings(QString("%1\\%2\\shell\\Open\\Command")
                                      .arg(REG_CLASSES_CLSID_PATH, guid),
                                  QSettings::NativeFormat);
        commandSettings.setValue(".", QString("%1 \"%1\"").arg(quotedAppPath)); // 启动命令

        // 在"此电脑"命名空间中注册
        QSettings namespaceSettings(QString("%1\\%2").arg(REG_COMPUTER_NAMESPACE_PATH, guid),
                                    QSettings::NativeFormat);
        namespaceSettings.setValue(".", appName); // 在"此电脑"中显示的名称

        return true;
    }
    // 取消注册：删除所有相关注册表项

    // 删除命名空间注册
    QSettings namespaceSettings(REG_COMPUTER_NAMESPACE_PATH, QSettings::NativeFormat);
    namespaceSettings.remove(guid);

    // 删除 CLSID 注册
    QSettings clsidSettings(REG_CLASSES_CLSID_PATH, QSettings::NativeFormat);
    clsidSettings.remove(guid);

    return true;
}

} // namespace Utils
