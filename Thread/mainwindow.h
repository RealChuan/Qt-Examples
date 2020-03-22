#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindowPrivate;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStart(bool);
    void onDoSomeThing();
    void setLable1(const QString&);
    void setLable2(const QString&);
    void setLable3(const QString&);

private:
    void buildConnect();
    void setupUI();
    MainWindowPrivate *d;
};
#endif // MAINWINDOW_H
