#include "mainwindow.h"
#include "logasync.h"

#include <QtConcurrent>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QTextEdit *edit = new QTextEdit(this);
    connect(LogAsync::instance(), &LogAsync::appendBuf, edit, &QTextEdit::append);
    setCentralWidget(edit);
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
    qDebug() << "Stop Log!";
}

void MainWindow::testLog()
{
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < 1000; i++) {
        if (!m_running)
            break;
        qInfo() << "1234567890qwertyuiopasdfghjklzxcvbnm" << i;
        QThread::msleep(1); //主界面无响应，上下文切换太快
    }

    qInfo() << timer.elapsed();
}
