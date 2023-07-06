#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QSqlError;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAdd();
    void onDelete();
    void onUpdate();
    void onSelect();
    void onRevert();
    void onShow();

private:
    void setupUI();
    void setupModel();
    void showError(const QSqlError &err);

    class MainWindowPrivate;
    QScopedPointer<MainWindowPrivate> d_ptr;
};
#endif // MAINWINDOW_H
