#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "MyThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_openbtn_clicked();
    void on_closebtn_clicked();
    void display(int i);

private:
    Ui::MainWindow *ui;
    MyThread *thread_;
    QTimer *time_;
    int count;
};
#endif // MAINWINDOW_H
