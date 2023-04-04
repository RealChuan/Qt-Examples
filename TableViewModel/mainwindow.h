#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "displaydata.hpp"

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

    DisplayTableView *m_table;
    QVector<DisplayInfo> m_datas;
};

#endif // MAINWINDOW_H
