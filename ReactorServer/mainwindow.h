#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindowPrivate;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onListen(bool);
    void onMaxCount(QAtomicInt);
    void onCount(QAtomicInt);

private:
    void setupUI();
    void initParam();
    void buildConnect();

    MainWindowPrivate *d;
};
#endif // MAINWINDOW_H
