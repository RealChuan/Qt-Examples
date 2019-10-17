#ifndef MAINWINDOW_OBJECT_H
#define MAINWINDOW_OBJECT_H

#include <QMainWindow>
#include "MyThread_object.h"

namespace Ui {
class MainWindow_object;
}

class MainWindow_object : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow_object(QWidget *parent = nullptr);
    ~MainWindow_object();

signals:
    void threadSignal();

private slots:
    void on_openbtn_clicked();
    void on_closebtn_clicked();
    void display(int i);

private:
    Ui::MainWindow_object *ui;
    QThread *thread_;
    MyObjectThread *objectThread_;
    QTimer *time_;
    int count;
};

#endif // MAINWINDOW_OBJECT_H
