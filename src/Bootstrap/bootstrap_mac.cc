#include "bootstrap.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>

namespace Bootstrap {

bool isAutoRunStart()
{
    QProcess process;
    process.start("launchctl", {"list", "com.myapp.launcher"});
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    qInfo() << output;
    bool loaded = !output.isEmpty();

    return loaded;
}

void setAutoRunStart(bool run)
{
    QString plistPath = QDir::homePath() + "/Library/LaunchAgents/com.myapp.launcher.plist";
    QFile plistFile(plistPath);
    QString sourcePath(":/resouce/com.myapp.launcher.plist");
    QFile sourceFile(sourcePath);
    if (plistFile.exists()) {
        qInfo() << plistFile.remove();
    }
    qInfo() << sourceFile.copy(plistPath) << sourceFile.error() << sourceFile.errorString();

    QProcess process;
    process.start("launchctl", {run ? "load" : "unload", "-w", plistPath});
    process.waitForFinished();
}

} // namespace Bootstrap
