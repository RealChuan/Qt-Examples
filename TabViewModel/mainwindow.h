#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class Student;
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
    StudentsTable* table;
    QList<Student*> students;
};
#endif // MAINWINDOW_H
