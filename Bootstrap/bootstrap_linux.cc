#include "bootstrap.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>

namespace Bootstrap {

bool isAutoRunStart()
{
    QProcess process;
    process.start("systemctl", {"--user", "status", "myapp.timer"});
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    bool disabled = output.contains("disabled");
    bool enabled = output.contains("enabled");
    //    bool active = output.contains("active");

    if (disabled) {
        return false;
    } else if (enabled) {
        return true;
    }
    return false;
}

void setAutoRunStart(bool run)
{
    QProcess process;
    process.start("mkdir", {"-p", QDir::homePath() + "/.config/systemd/user"});
    //process.start("systemctl", {"--user"});
    process.waitForFinished();
    // service
    QString servicePath = QDir::homePath() + "/.config/systemd/user/myapp.service";
    QFile serviceFile(servicePath);
    QString sourcePath(":/resouce/myapp.service");
    QFile sourceFile(sourcePath);
    if (serviceFile.exists()) {
        qInfo() << serviceFile.remove();
    }
    qInfo() << sourceFile.copy(servicePath) << sourceFile.error() << sourceFile.errorString();
    // timer 等待图形界面启动后再启动，否则会出现qxcbconnection: could not connect to display
    QString timerPath = QDir::homePath() + "/.config/systemd/user/myapp.timer";
    QFile timerFile(timerPath);
    QString timerSourcePath = ":/resouce/myapp.timer";
    QFile timerSourceFile(timerSourcePath);
    if (timerSourceFile.exists()) {
        qInfo() << timerSourceFile.remove();
    }
    qInfo() << timerSourceFile.copy(timerPath) << timerSourceFile.error()
            << timerSourceFile.errorString();

    process.start("systemctl", {"--user", (run ? "enable" : "disable"), "myapp.timer"});
    process.waitForFinished();
}

} // namespace Bootstrap
