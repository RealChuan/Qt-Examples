#include "mainwindow.h"
#include "testtable.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    init();
}

MainWindow::~MainWindow()
{
    qDeleteAll(students);
    students.clear();
}

void MainWindow::onAdd()
{
    Student *stu = new Student(this);
    stu->setID(quint16(students.size()));
    students.append(stu);
    table->setStudents(students);
    table->selectRow(students.size() -1);
}

void MainWindow::onRemove()
{
    int row = table->currentIndex().row();
    if(row<0 || row>students.size())
        return;
    delete students.takeAt(row);
    table->setStudents(students);
    table->selectRow(students.size() -1);
}

void MainWindow::init()
{
    students.append(new Student(0, "Jason",     15, "MALE",   66, this));
    students.append(new Student(1, "Lily",      13, "FEMALE", 85, this));
    students.append(new Student(2, "Odin",      16, "FEMALE", 76, this));
    students.append(new Student(3, "Willion",   12, "MALE",   89, this));
    students.append(new Student(4, "Nieo",      14, "MALE",   77, this));
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
    setMinimumSize(600, 260);

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(removeBtn, &QPushButton::clicked, this, &MainWindow::onRemove);
}

