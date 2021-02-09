#include "mainwindow.h"
#include "stuedenttable.h"
#include "student.h"
#include "studenttablemodel.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    init();
    resize(800, 300);
}

MainWindow::~MainWindow()
{
    if(!students.isEmpty()){
        qDeleteAll(students);
        students.clear();
    }
}

void MainWindow::onInsertItem()
{
    int row = table->currentIndex().row();
    Student *stu = new Student;
    stu->id = students.size();

    if(row < 0 || row >= students.size())
        students.append(stu);
    else
        students.insert(row, stu);

    table->setStudents(students);
}

void MainWindow::onRemoveItem()
{
    QModelIndex index = table->currentIndex();
    if(!index.isValid())
        return;
    int row = index.row();
    delete students.takeAt(row);
    table->setStudents(students);
}

void MainWindow::init()
{
    students.append(new Student{true, 0, "Jason", 15, "MALE", 66, 10});
    students.append(new Student{false, 1, "Lily", 13, "FEMALE", 85, 20});
    students.append(new Student{true, 2, "Odin", 16, "FEMALE", 76, 30});
    students.append(new Student{false, 3, "Willion", 12, "MALE", 89, 40});
    students.append(new Student{true, 4, "Nieo", 14, "MALE", 77, 50});
    table->setStudents(students);
    table->selectRow(students.size() - 1);
}

void MainWindow::setupUI()
{
    QPushButton *addBtn = new QPushButton(tr("Add"), this);
    QPushButton *removeBtn = new QPushButton(tr("Remove"), this);
    table = new StudentsTable(this);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch(1);
    hLayout->addWidget(addBtn);
    hLayout->addWidget(removeBtn);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(hLayout);
    layout->addWidget(table);
    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);
    setCentralWidget(frame);

    connect(table, &StudentsTable::insertItem, this, &MainWindow::onInsertItem);
    connect(table, &StudentsTable::removeItem, this, &MainWindow::onRemoveItem);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onInsertItem);
    connect(removeBtn, &QPushButton::clicked, this, &MainWindow::onRemoveItem);
}
