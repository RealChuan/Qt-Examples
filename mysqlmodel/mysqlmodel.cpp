#include "mysqlmodel.h"
#include "ui_mysqlmodel.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QMessageBox>

#pragma execution_character_set("utf-8")

mysqlmodel::mysqlmodel(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::mysqlmodel)
{
    ui->setupUi(this);
    openmysql();

    model = new QSqlTableModel(this);
    model->setTable("students");
    model->select();
    // 设置编辑策略
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    ui->display->setModel(model);

}

mysqlmodel::~mysqlmodel()
{
    db.close();
    delete ui;
}

void mysqlmodel::openmysql()
{
    db=QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");      //连接数据库主机名，这里需要注意（若填的为”127.0.0.1“，出现不能连接，则改为localhost)
    db.setPort(3306);                 //连接数据库端口号，与设置一致
    db.setDatabaseName("school");     //连接数据库名，与设置一致
    db.setUserName("root");           //数据库用户名，与设置一致
    db.setPassword("root");           //数据库密码，与设置一致
    if(!db.open())
    {
        qDebug()<<"不能连接"<<"connect to mysql error"<<db.lastError().text();
        return ;
    }
    else
    {
        qDebug()<<"连接成功"<<"connect to mysql OK";
    }
}

void mysqlmodel::on_insert_clicked()
{
    // 获得表的行数
    int rowNum = model->rowCount();
    QSqlQuery query(db);
    query.exec("SELECT id FROM students");
    int id=0;
    if(query.last()){
        id=query.value(0).toInt();
        qDebug()<<id<<endl;
    }
    // 添加一行
    model->insertRow(rowNum);
    model->setData(model->index(rowNum,0),id+1);
}

void mysqlmodel::on_del_clicked()
{
    // 获取选中的行
    int curRow = ui->display->currentIndex().row();
    // 删除该行
    model->removeRow(curRow);
    int ok = QMessageBox::warning(this,tr("删除当前行!"),
                                  tr("你确定删除当前行吗？ "),
                                  QMessageBox::Yes, QMessageBox::No);
    if(ok == QMessageBox::No)
        model->revertAll();// 如果不删除， 则撤销
    else
        model->submitAll();// 否则提交， 在数据库中删除该行
}

void mysqlmodel::on_update_clicked()
{
    // 开始事务操作
    model->database().transaction();
    if (model->submitAll())
        model->database().commit(); //提交
    else
    {
        model->database().rollback(); //回滚
        QMessageBox::warning(this, tr("tableModel"),
                             tr("数据库错误: %1").
                             arg(model->lastError().text()));
    }
}

void mysqlmodel::on_rollback_clicked()
{
    model->revertAll();
}

void mysqlmodel::on_showlist_clicked()
{
    model->setTable("students");
    model->select();
}

void mysqlmodel::on_select_clicked()
{
    QString name = ui->name->text();
    if(name.length())
    {
        //根据姓名进行筛选， 一定要使用单引号
        model->setFilter(QString("name = '%1'").arg(name));
        model->select();
    }
    else
        QMessageBox::warning(this, tr("Warning"),tr("姓名不能为空") );
}
