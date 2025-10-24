#pragma once

#include <QMainWindow>

class DisplayTableView;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onInsertItem();
    void onRemoveItem();

private:
    void init();
    void setupUI();

    class MainWindowPrivate;
    QScopedPointer<MainWindowPrivate> d_ptr;
};
