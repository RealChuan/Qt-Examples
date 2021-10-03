#include "mainwindow.h"
#include "thread1.h"
#include "thread2.h"
#include "thread3.h"

#include <QtWidgets>
#include <QtConcurrent>

QLabel* createLabel(QWidget *parent)
{
    QLabel *label = new QLabel(parent);
    label->setAlignment(Qt::AlignCenter);
    return label;
}

class MainWindowPrivate{
public:
    MainWindowPrivate(QWidget *parent)
        : owner(parent)
        , count(0){
        mainLabel = createLabel(owner);
        threadLabel1 = createLabel(owner);
        threadLabel2 = createLabel(owner);
        threadLabel3 = createLabel(owner);
        threadLabel4 = createLabel(owner);
        startButton = new QPushButton(QObject::tr("Start Threads"), owner);
        startButton->setCheckable(true);
        doButton = new QPushButton(QObject::tr("Do Something"), owner);
        doButton->setEnabled(false);
    }
    QWidget *owner;
    QLabel *mainLabel;
    QLabel *threadLabel1;
    QLabel *threadLabel2;
    QLabel *threadLabel3;
    QLabel *threadLabel4;
    QPushButton *startButton;
    QPushButton *doButton;
    Thread1 *thread1;
    Thread2 *thread2;
    Thread3 *thread3;
    int count;

    QFuture<void> wacher;
    QMutex mutex;
    QWaitCondition waitCondition;
    volatile bool runing = true;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d(new MainWindowPrivate(this))
{
    setupUI();
    buildConnect();
    resize(640, 480);
}

MainWindow::~MainWindow()
{
    d->runing = false;
    d->waitCondition.wakeOne();
    d->wacher.waitForFinished();
}

void MainWindow::onStart(bool checked)
{
    if(checked){
        d->thread1 = new Thread1(this);
        d->thread2 = new Thread2(this);
        d->thread3 = new Thread3(this);

        // Qt::DirectConnection直连，QLabel支持子线程中更新，（部分控件是支持在子线程中更新）
        connect(d->thread1, &Thread1::message, this, &MainWindow::onLabel1/*, Qt::DirectConnection*/);
        connect(d->thread2, &Thread2::message, this, &MainWindow::onLabel2, Qt::DirectConnection);
        connect(d->thread3, &Thread3::message, this, &MainWindow::onLabel3, Qt::DirectConnection);
        d->wacher = QtConcurrent::run(&MainWindow::onRunCurrent, this, 4);

        d->thread1->start();
        d->thread2->start();

        d->startButton->setText(tr("Stop Threads"));
        d->doButton->setEnabled(true);
    }else{
        d->thread1->deleteLater();
        d->thread2->deleteLater();
        d->thread3->deleteLater();
        d->runing = false;
        d->waitCondition.wakeOne();
        d->startButton->setText(tr("Start Threads"));
        d->doButton->setEnabled(false);
    }
}

void MainWindow::onDoSomeThing()
{
    d->thread1->wakeUpThread();
    emit d->thread2->doSomeThing(2);
    emit d->thread3->doSomething(3);
    //d->thread3->doSomeThing(3);
    d->waitCondition.wakeOne();
    QString str = tr("I am main thread(UI): %1, Count: %2").
                  arg(reinterpret_cast<qlonglong>(QThread::currentThreadId())).
                  arg(d->count);
    d->mainLabel->setText(str);
    d->count++;
    update();
}

void MainWindow::onLabel1(const QString &str)
{
    QString buf = str + "\n" + tr("The current thread is: %1").
                               arg(reinterpret_cast<qlonglong>(QThread::currentThreadId()));
    d->threadLabel1->setText(buf);
}

void MainWindow::onLabel2(const QString &str)
{
    QString buf = str + "\n" + tr("The current thread is: %1").
                               arg(reinterpret_cast<qlonglong>(QThread::currentThreadId()));
    d->threadLabel2->setText(buf);
}

void MainWindow::onLabel3(const QString &str)
{
    QString buf = str + "\n" + tr("The current thread is: %1").
                               arg(reinterpret_cast<qlonglong>(QThread::currentThreadId()));
    d->threadLabel3->setText(buf);
}

void MainWindow::onLabel4(const QString &str)
{
    QString buf = str + "\n" + tr("The current thread is: %1").
                               arg(reinterpret_cast<qlonglong>(QThread::currentThreadId()));
    d->threadLabel4->setText(buf);
}

void MainWindow::onRunCurrent(const int index)
{
    int count = 0;
    d->runing = true;
    while(d->runing){
        QMutexLocker locker(&d->mutex);
        d->waitCondition.wait(&d->mutex);
        QString str = tr("I am thread %1: %2, Count: %3").arg(index).
                      arg(reinterpret_cast<qlonglong>(QThread::currentThreadId())).
                      arg(count);
        count++;
        onLabel4(str);
    }
}

void MainWindow::buildConnect()
{
    connect(d->startButton, &QPushButton::clicked, this, &MainWindow::onStart);
    connect(d->doButton, &QPushButton::clicked, this, &MainWindow::onDoSomeThing);
}

void MainWindow::setupUI()
{
    QWidget *widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(widget);
    layout->addWidget(d->mainLabel, 0, 0, 1 ,2);
    layout->addWidget(d->threadLabel1, 1, 0);
    layout->addWidget(d->threadLabel2, 1, 1);
    layout->addWidget(d->threadLabel3, 2, 0);
    layout->addWidget(d->threadLabel4, 2, 1);
    layout->addWidget(d->startButton, 3, 0);
    layout->addWidget(d->doButton, 3, 1);

    setCentralWidget(widget);

    setStyleSheet("QLabel{background: #787878;border-radius: 4px;"
                  "font-family: \"Microsoft YaHei\";font-weight:bold;"
                  "font-size:14px;color: #FFFFFF;}");
}

