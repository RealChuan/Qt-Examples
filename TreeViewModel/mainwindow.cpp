#include "mainwindow.h"
#include "studentmodel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    TreeView *treeView = new TreeView(this);
    setCentralWidget(treeView);

    StuVec stuVec;
    for(quint16 i= 0; i < 5; i++){
        StudentItem item(i, "John", 18, "Male", 60);
        stuVec.append(item);
    }

    QVector<ClassPair> classVec;
    for(quint16 i= 0; i < 5; i++){
        StudentItem item(i, "Class", 0, "", 0);
        ClassPair classPair(item, stuVec);
        classVec.append(classPair);
    }
    treeView->updateValueTree(classVec);
    resize(1000, 618);
}

MainWindow::~MainWindow()
{
}
