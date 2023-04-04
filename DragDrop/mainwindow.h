#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class menuWidget;
class acceptWidget;
class QTextEdit;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void setupUI();
    QTextEdit *textEdit;
};
#endif // MAINWINDOW_H
