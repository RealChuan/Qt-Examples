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
    void onRequest();
    void onFinish();
    void onAppendBytes(const QByteArray&);
    void onAppendText();

private:
    void setupUI();
    void buildConnect();

    QScopedPointer<MainWindowPrivate> d;
};
#endif // MAINWINDOW_H
