#include "autostartmanager.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>

using namespace Qt::StringLiterals;

namespace Utils {

// 获取 LaunchAgent 服务名与 plist 路径
static QString launchServiceName()
{ return u"com.%1.launcher"_s.arg(QCoreApplication::applicationName().toLower()); }

static QString plistPath()
{ return QDir::homePath() + u"/Library/LaunchAgents/%1.plist"_s.arg(launchServiceName()); }

[[nodiscard]] std::expected<bool, QString> isAutoRunStart()
{
    // 通过 plist 文件是否存在判断自启动配置（launchctl list 反映的是运行状态）
    return QFile::exists(plistPath());
}

[[nodiscard]] std::expected<void, QString> setAutoRunStart(bool run)
{
    const QString path = plistPath();

    if (run) {
        // 确保目录存在
        const QDir launchAgentsDir(QDir::homePath() + u"/Library/LaunchAgents"_s);
        if (!launchAgentsDir.exists() && !launchAgentsDir.mkpath(u"."_s)) {
            return std::unexpected(u"Failed to create LaunchAgents directory"_s);
        }

        // 动态生成 plist 内容
        const QString plistContent = uR"(<?xml version="1.0" encoding="UTF-8"?>
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
)"_s.arg(launchServiceName(), QCoreApplication::applicationFilePath());

        // 写入 plist 文件
        QFile plistFile(path);
        if (!plistFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return std::unexpected(
                u"Failed to open plist file for writing: %1"_s.arg(plistFile.errorString()));
        }
        if (plistFile.write(plistContent.toUtf8()) == -1) {
            return std::unexpected(
                u"Failed to write plist file: %1"_s.arg(plistFile.errorString()));
        }
        plistFile.close();

        // 加载服务
        QProcess process;
        process.start(u"launchctl"_s, {u"load"_s, u"-w"_s, path});
        if (!process.waitForFinished(5000)) {
            return std::unexpected(u"launchctl load timeout"_s);
        }
        if (process.exitCode() != 0) {
            return std::unexpected(u"launchctl load failed: %1"_s.arg(
                QString::fromUtf8(process.readAllStandardError())));
        }
    } else {
        // 先卸载服务，再删除 plist 文件（修复原实现先写 plist 再 unload 的 Bug）
        QProcess process;
        process.start(u"launchctl"_s, {u"unload"_s, u"-w"_s, path});
        if (!process.waitForFinished(5000)) {
            return std::unexpected(u"launchctl unload timeout"_s);
        }
        if (process.exitCode() != 0) {
            qWarning() << "launchctl unload failed:" << process.readAllStandardError();
            // 即使卸载失败也尝试删除 plist，避免残留
        }

        if (QFile::exists(path) && !QFile::remove(path)) {
            return std::unexpected(u"Failed to remove plist file"_s);
        }
    }

    qInfo() << "Auto-run" << (run ? "enabled" : "disabled") << "successfully";
    return {};
}

} // namespace Utils
