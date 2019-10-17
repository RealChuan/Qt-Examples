#ifndef MYSQLMODEL_H
#define MYSQLMODEL_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>

namespace Ui {
class mysqlmodel;
}

class mysqlmodel : public QMainWindow
{
    Q_OBJECT

public:
    explicit mysqlmodel(QWidget *parent = nullptr);
    ~mysqlmodel();

private:
    Ui::mysqlmodel *ui;

    QSqlDatabase db;
    QSqlTableModel *model;

private slots:
    void openmysql();
    void on_insert_clicked();
    void on_del_clicked();
    void on_update_clicked();
    void on_rollback_clicked();
    void on_showlist_clicked();
    void on_select_clicked();
};

#endif // MYSQLMODEL_H
