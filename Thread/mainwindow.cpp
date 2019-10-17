#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTimer>

#pragma execution_character_set("utf-8")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,count(0)
{
    ui->setupUi(this);

    ui->label->setFont(QFont("微软雅黑", 20, QFont::Normal, false));
    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->setText("Hello World!");

    ui->label_3->setFont(QFont("微软雅黑", 20, QFont::Normal, false));
    ui->label_3->setAlignment(Qt::AlignCenter);
    ui->label_3->setText("Hello World!");

    time_ = new QTimer(this);
    time_->setInterval(1000);
    connect(time_,&QTimer::timeout,[this]
    {
        QString str=QString("UI线程函数\n编号：%1\n线程id:%2").arg(count++)
                .arg(reinterpret_cast<int>(QThread::currentThreadId()));
        ui->label->setText(str);
        qDebug()<<"主线程："<<QThread::currentThreadId();
    });

    /*线程初始化*/
    thread_ = new MyThread;
    connect(thread_,&MyThread::message,this,&MainWindow::display,Qt::DirectConnection);

    connect(thread_,&QThread::finished,[]{
        qDebug()<<"线程："<<QThread::currentThreadId();
        qDebug()<<tr("完成信号finished触发");
    });
}

MainWindow::~MainWindow()
{
    thread_->closeThread();
    thread_->wait();
    thread_->deleteLater();
    delete thread_;
    delete ui;
}

void MainWindow::on_openbtn_clicked()
{
    time_->start();
    /*开启一个线程*/
    qDebug()<<"主线程："<<QThread::currentThreadId();
    thread_->setRun(true);
    thread_->start();
}

void MainWindow::on_closebtn_clicked()
{
    time_->stop();
    /*关闭多线程*/
    thread_->closeThread();
    thread_->wait();
}

void MainWindow::display(int i)
{
    QString str=QString("信号槽常规函数\n编号：%1\n线程id:%2").arg(i)
            .arg(reinterpret_cast<int>(QThread::currentThreadId()));
    ui->label_3->setText(str);
}
