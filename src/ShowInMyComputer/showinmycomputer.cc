#include "showinmycomputer.hpp"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QSettings>

static const auto GUID = "{789183FC-5E6B-4948-8C18-7AF0BBE527A1}";
static const auto REG_COMPUTERNAMESPACE_PATH
    = "HKEY_CURRENT_"
      "USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace";
static const auto REG_CLASS_CLSID_PATH = "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\CLSID";

bool ShowInMyComputerme::isShowInMyComputer()
{
    QSettings settings1(REG_CLASS_CLSID_PATH, QSettings::NativeFormat);
    if (!settings1.childGroups().contains(GUID)) {
        return false;
    }
    QSettings settings4(REG_COMPUTERNAMESPACE_PATH, QSettings::NativeFormat);
    if (!settings4.childGroups().contains(GUID)) {
        return false;
    }
    return true;
}

void ShowInMyComputerme::setShowInMyComputer(bool show)
{
    static auto appName = QApplication::applicationName();
    static auto appPath = QString("\"%1\"").arg(
        QDir::toNativeSeparators(qApp->applicationFilePath()));

    if (show) {
        QSettings settings1(QString("%1\\%2").arg(REG_CLASS_CLSID_PATH, GUID),
                            QSettings::NativeFormat);
        settings1.setValue(".", appName);
        settings1.setValue("InfoTip", appName);

        QSettings settings2(QString("%1\\%2\\DefaultIcon").arg(REG_CLASS_CLSID_PATH, GUID),
                            QSettings::NativeFormat);
        settings2.setValue(".", appPath);

        QSettings settings3(QString("%1\\%2\\shell\\Open\\Command").arg(REG_CLASS_CLSID_PATH, GUID),
                            QSettings::NativeFormat);
        settings3.setValue(".", appPath);

        QSettings settings4(QString("%1\\%2").arg(REG_COMPUTERNAMESPACE_PATH, GUID),
                            QSettings::NativeFormat);
        settings4.setValue(".", appName);
    } else {
        QSettings settings1(REG_CLASS_CLSID_PATH, QSettings::NativeFormat);
        settings1.remove(GUID);

        QSettings settings4(REG_COMPUTERNAMESPACE_PATH, QSettings::NativeFormat);
        settings4.remove(GUID);
    }
}
