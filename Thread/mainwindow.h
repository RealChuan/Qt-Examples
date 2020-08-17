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
    void onLabel1(const QString&);
    void onLabel2(const QString&);
    void onLabel3(const QString&);
    void onLabel4(const QString&);
    void onRunCurrent(const int index);

private:
    void buildConnect();
    void setupUI();

    QScopedPointer<MainWindowPrivate> d;
};
#endif // MAINWINDOW_H
