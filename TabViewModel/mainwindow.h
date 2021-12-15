#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "student.h"

class StudentsTable;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onInsertItem();
    void onRemoveItem();

private:
    void init();
    void setupUI();
    StudentsTable *m_table;
    QVector<Student> m_students;
};
#endif // MAINWINDOW_H
