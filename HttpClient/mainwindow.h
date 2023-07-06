#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onRequest();
    void onFinish();

private:
    void setupUI();
    void buildConnect();

    class MainWindowPrivate;
    QScopedPointer<MainWindowPrivate> d_ptr;
};
#endif // MAINWINDOW_H
