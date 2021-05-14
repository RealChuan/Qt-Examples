#include "mainwindow.h"
#include "student.h"
#include "studenttablemodel.h"
#include "stuedenttable.h"

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
    if (!m_students.isEmpty()) {
        qDeleteAll(m_students);
        m_students.clear();
    }
}

void MainWindow::onInsertItem()
{
    int row = m_table->currentIndex().row();
    Student *stu = new Student;
    stu->id = m_students.size();

    if (row < 0 || row >= m_students.size())
        m_students.append(stu);
    else
        m_students.insert(row, stu);

    m_table->setStudents(m_students);
}

void MainWindow::onRemoveItem()
{
    QModelIndex index = m_table->currentIndex();
    if (!index.isValid())
        return;
    int row = index.row();
    delete m_students.takeAt(row);
    m_table->setStudents(m_students);
}

void MainWindow::init()
{
    m_students.append(new Student{true, 0, "Jason", 15, "MALE", 66, 10});
    m_students.append(new Student{false, 1, "Lily", 13, "FEMALE", 85, 20});
    m_students.append(new Student{true, 2, "Odin", 16, "FEMALE", 76, 30});
    m_students.append(new Student{false, 3, "Willion", 12, "MALE", 89, 40});
    m_students.append(new Student{true, 4, "Nieo", 14, "MALE", 77, 50});
    m_table->setStudents(m_students);
    m_table->selectRow(m_students.size() - 1);
}

void MainWindow::setupUI()
{
    QPushButton *addBtn = new QPushButton(tr("Add"), this);
    QPushButton *removeBtn = new QPushButton(tr("Remove"), this);
    m_table = new StudentsTable(this);
    ;
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch(1);
    hLayout->addWidget(addBtn);
    hLayout->addWidget(removeBtn);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(hLayout);
    layout->addWidget(m_table);
    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);
    setCentralWidget(frame);

    connect(m_table, &StudentsTable::insertItem, this, &MainWindow::onInsertItem);
    connect(m_table, &StudentsTable::removeItem, this, &MainWindow::onRemoveItem);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onInsertItem);
    connect(removeBtn, &QPushButton::clicked, this, &MainWindow::onRemoveItem);
}
