#include "mainwindow_object.h"
#include "ui_mainwindow_object.h"

#include <QThread>
#include <QTimer>
#include <QDebug>

#pragma execution_character_set("utf-8")

MainWindow_object::MainWindow_object(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow_object),
    count(0)
{
    ui->setupUi(this);
    ui->label->setFont(QFont("微软雅黑", 20, QFont::Normal, false));
    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->setText("Hello World!");

    ui->label_3->setFont(QFont("微软雅黑", 20, QFont::Normal, false));
    ui->label_3->setAlignment(Qt::AlignCenter);
    ui->label_3->setText("Hello World!");

    qDebug()<<"主线程："<<QThread::currentThreadId();

    time_ = new QTimer(this);
    time_->setInterval(1000);
    connect(time_,&QTimer::timeout,[this]
    {
        QString str=QString("UI线程函数\n编号：%1\n线程id:%2").arg(count++)
                .arg(reinterpret_cast<int>(QThread::currentThreadId()));
        ui->label->setText(str);
        qDebug()<<"主线程："<<QThread::currentThreadId();
    });

    thread_=new QThread;
    objectThread_=new MyObjectThread;
    objectThread_->moveToThread(thread_);
    thread_->start();

    //qRegisterMetaType注册自定义类型
    connect(this,&MainWindow_object::threadSignal,objectThread_,&MyObjectThread::test);

    //信号和槽函数在同一个线程
    connect(objectThread_,&MyObjectThread::message,this,&MainWindow_object::display,Qt::DirectConnection);

    connect(thread_,&QThread::finished,thread_,&QThread::deleteLater);
}

MainWindow_object::~MainWindow_object()
{
    delete ui;
}

void MainWindow_object::on_openbtn_clicked()
{
    time_->start();
    objectThread_->setRun(true);
    emit threadSignal();
    emit objectThread_->selfSignal();
}

void MainWindow_object::on_closebtn_clicked()
{
    time_->stop();
    objectThread_->setRun(false);
}

void MainWindow_object::display(int i)
{
    QString str=QString("信号槽常规函数\n编号：%1\n线程id:%2").arg(i)
            .arg(reinterpret_cast<int>(QThread::currentThreadId()));
    ui->label_3->setText(str);
}
