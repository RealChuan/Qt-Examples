#include "autostartmanager.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>

namespace Utils {

bool isAutoRunStart()
{
    QString appName = qApp->applicationName();
    QString launchServiceName = QString("com.%1.launcher").arg(appName.toLower());

    QProcess process;
    process.start("launchctl", {"list", launchServiceName});

    if (!process.waitForFinished(3000)) {
        qWarning() << "launchctl list timeout";
        return false;
    }

    return (process.exitCode() == 0);
}

void setAutoRunStart(bool run)
{
    QString appName = qApp->applicationName();
    QString launchServiceName = QString("com.%1.launcher").arg(appName.toLower());
    QString plistPath = QDir::homePath()
                        + QString("/Library/LaunchAgents/%1.plist").arg(launchServiceName);

    // 动态生成plist内容
    QString plistContent = QString(R"(
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>%1</string>
    <key>ProgramArguments</key>
    <array>
        <string>%2</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
</dict>
</plist>
)")
                               .arg(launchServiceName, QCoreApplication::applicationFilePath());

    // 确保目录存在
    QDir launchAgentsDir(QDir::homePath() + "/Library/LaunchAgents");
    if (!launchAgentsDir.exists() && !launchAgentsDir.mkpath(".")) {
        qWarning() << "Failed to create LaunchAgents directory";
        return;
    }

    // 写入plist文件
    QFile plistFile(plistPath);
    if (!plistFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open plist file for writing:" << plistFile.errorString();
        return;
    }

    if (plistFile.write(plistContent.toUtf8()) == -1) {
        qWarning() << "Failed to write plist file:" << plistFile.errorString();
        plistFile.close();
        return;
    }
    plistFile.close();

    // 加载或卸载服务
    QProcess process;
    process.start("launchctl", {run ? "load" : "unload", "-w", plistPath});

    if (!process.waitForFinished(5000)) {
        qWarning() << "launchctl operation timeout";
        return;
    }

    if (process.exitCode() != 0) {
        qWarning() << "launchctl failed with exit code:" << process.exitCode();
        qWarning() << "Error:" << process.readAllStandardError();
    } else {
        qInfo() << "Auto-run" << (run ? "enabled" : "disabled") << "successfully";
    }
}

} // namespace Utils
