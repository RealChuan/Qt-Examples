#include "mainwindow.h"
#include "thread1.h"
#include "thread2.h"
#include "thread3.h"

#include <QtWidgets>

class MainWindowPrivate{
public:
    MainWindowPrivate(QWidget *parent)
        :owner(parent)
        ,count(0){
        mainLabel = new QLabel(owner);
        threadLabel1 = new QLabel(owner);
        threadLabel1->setAlignment(Qt::AlignCenter);
        threadLabel2 = new QLabel(owner);
        threadLabel2->setAlignment(Qt::AlignCenter);
        threadLabel3 = new QLabel(owner);
        threadLabel3->setAlignment(Qt::AlignCenter);
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
    QPushButton *startButton;
    QPushButton *doButton;
    Thread1 *thread1;
    Thread2 *thread2;
    Thread3 *thread3;
    int count;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d(new MainWindowPrivate(this))
{
    setupUI();
    buildConnect();
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::onStart(bool checked)
{
    if(checked){
        d->thread1 = new Thread1(this);
        d->thread2 = new Thread2(this);
        d->thread3 = new Thread3(this);

        // Qt::DirectConnection直连，QLabel支持子线程中更新，（部分控件是支持在子线程中更新）
        connect(d->thread1, &Thread1::message, this, &MainWindow::setLable1/*, Qt::DirectConnection*/);
        connect(d->thread2, &Thread2::message, this, &MainWindow::setLable2, Qt::DirectConnection);
        connect(d->thread3, &Thread3::message, this, &MainWindow::setLable3, Qt::DirectConnection);

        d->thread1->start();
        d->thread2->start();

        d->startButton->setText(tr("Stop Threads"));
        d->doButton->setEnabled(true);
    }else{
        d->thread1->deleteLater();
        d->thread2->deleteLater();
        d->thread3->deleteLater();
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
    QString str = tr("I am main thread(UI): %1, Count: %2").
            arg(reinterpret_cast<int>(QThread::currentThreadId())).
            arg(d->count);
    d->mainLabel->setText(str);
    d->count++;
    update();
}

void MainWindow::setLable1(const QString &str)
{
    QString buf = str + "\n" + tr("The current thread is: %1").
            arg(reinterpret_cast<int>(QThread::currentThreadId()));
    d->threadLabel1->setText(buf);
}

void MainWindow::setLable2(const QString &str)
{
    QString buf = str + "\n" + tr("The current thread is: %1").
            arg(reinterpret_cast<int>(QThread::currentThreadId()));
    d->threadLabel2->setText(buf);
}

void MainWindow::setLable3(const QString &str)
{
    QString buf = str + "\n" + tr("The current thread is: %1").
            arg(reinterpret_cast<int>(QThread::currentThreadId()));
    d->threadLabel3->setText(buf);
}

void MainWindow::buildConnect()
{
    connect(d->startButton, &QPushButton::clicked, this, &MainWindow::onStart);
    connect(d->doButton, &QPushButton::clicked, this, &MainWindow::onDoSomeThing);
}

void MainWindow::setupUI()
{
    QGridLayout *gLayout = new QGridLayout;
    gLayout->addWidget(d->mainLabel, 0, 0);
    gLayout->addWidget(d->threadLabel1, 0, 1);
    gLayout->addWidget(d->threadLabel2, 1, 0);
    gLayout->addWidget(d->threadLabel3, 1, 1);
    gLayout->addWidget(d->startButton, 2, 0);
    gLayout->addWidget(d->doButton, 2, 1);

    QFrame *frame = new QFrame(this);
    frame->setLayout(gLayout);
    setCentralWidget(frame);
    setMinimumSize(640, 480);

    setStyleSheet("QLabel{\
                  background-color: #787878;\
                  border-radius: 4px;\
                  font-family: \"Microsoft YaHei\";\
                  font-weight:bold;\
                  font-size:14px;\
                  color: #FFFFFF;\
                  }");
}

