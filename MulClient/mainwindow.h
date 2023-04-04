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
    void onConnect(bool);
    void onWrite();
    void removeOne(int index);

private:
    void warningBox(const QString&, QWidget*);
    void buildConnect();
    void setupUI();
    void changeControlState(bool);
    void clearAll();

    MainWindowPrivate *d;
};
#endif // MAINWINDOW_H
