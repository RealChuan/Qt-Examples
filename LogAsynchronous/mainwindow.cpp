#include "mainwindow.h"
#include "logasync.h"

#include <QtConcurrent>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto *log = LogAsync::instance();
    log->setLogPath(QDir::tempPath());
    log->setAutoDelFile(true);
    log->setAutoDelFileDays(7);
    log->setOrientation(LogAsync::Orientation::StandardAndFile);
    log->setLogLevel(QtDebugMsg);
    log->startWork();

    auto *textBrowser = new QTextBrowser(this);
    connect(log, &LogAsync::appendBuf, textBrowser, &QTextBrowser::append);
    setCentralWidget(textBrowser);
    resize(1000, 618);
    qDebug() << "Start Log!";
    m_watcher = QtConcurrent::run(&MainWindow::testLog, this);
}

MainWindow::~MainWindow()
{
    m_running = false;
    if (m_watcher.isRunning()) {
        m_watcher.cancel();
        m_watcher.waitForFinished();
    }
    LogAsync::instance()->stop();
    qDebug() << "Stop Log!";
}

void MainWindow::testLog()
{
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < 1000; i++) {
        if (!m_running) {
            break;
        }
        qInfo() << "1234567890qwertyuiopasdfghjklzxcvbnm" << i;
        QThread::msleep(100); //主界面无响应，上下文切换太快
    }

    qInfo() << timer.elapsed();
}
