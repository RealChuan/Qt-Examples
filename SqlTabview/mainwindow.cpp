#include "mainwindow.h"
#include "sql.h"

#include <QMessageBox>
#include <QtSql>
#include <QtWidgets>

class MainWindow::MainWindowPrivate
{
public:
    explicit MainWindowPrivate(QWidget *parent)
        : d_ptr(parent)
    {
        sqlTabView = new QTableView(d_ptr);
        sqlTabView->setAlternatingRowColors(true);
        sqlTabView->horizontalHeader()->setStretchLastSection(true);
        lineEdie = new QLineEdit(d_ptr);
    }

    QWidget *d_ptr;

    Sql *sqlite;
    QSqlTableModel *sqlModel;
    QTableView *sqlTabView;
    QLineEdit *lineEdie;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new MainWindowPrivate(this))
{
    if (!Sql::searchSQLite()) {
        return;
    }
    d_ptr->sqlite = new Sql(this);
    QSqlError error = d_ptr->sqlite->createOrOpenSqlite();
    if (error.type() != QSqlError::NoError) {
        showError(error);
        return;
    }
    setupUI();
    setupModel();

    resize(1000, 618);
}

MainWindow::~MainWindow() {}

void MainWindow::onAdd()
{
    // 获得表的行数
    int rowNum = d_ptr->sqlModel->rowCount();
    QSqlQuery query;
    int id = query.lastInsertId().toInt();
    // 添加一行
    d_ptr->sqlModel->insertRow(rowNum);
    d_ptr->sqlModel->setData(d_ptr->sqlModel->index(rowNum, 0), id + 1);
}

void MainWindow::onDelete()
{
    int curRow = d_ptr->sqlTabView->currentIndex().row();
    d_ptr->sqlModel->removeRow(curRow);
    int ok = QMessageBox::warning(this,
                                  tr("Delete current row!"),
                                  tr("Are you sure you want to delete the current line?"),
                                  QMessageBox::Yes,
                                  QMessageBox::No);
    if (ok == QMessageBox::No) {
        d_ptr->sqlModel->revertAll();
    }                                 // 如果不删除， 则撤销
    else {
        d_ptr->sqlModel->submitAll(); // 否则提交， 在数据库中删除该行
    }
}
void MainWindow::onUpdate()
{
    // 开始事务操作
    d_ptr->sqlModel->database().transaction();
    if (d_ptr->sqlModel->submitAll()) {
        d_ptr->sqlModel->database().commit();   //提交
    } else {
        d_ptr->sqlModel->database().rollback(); //回滚
        QMessageBox::warning(this,
                             tr("tableModel"),
                             tr("Database error: %1").arg(d_ptr->sqlModel->lastError().text()));
    }
}

void MainWindow::onSelect()
{
    QString name = d_ptr->lineEdie->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Name cannot be empty!"));
    } else {
        //根据姓名进行筛选， 一定要使用单引号
        d_ptr->sqlModel->setFilter(QString("Name = '%1'").arg(name));
        d_ptr->sqlModel->select();
    }
}

void MainWindow::onRevert()
{
    d_ptr->sqlModel->revertAll();
}

void MainWindow::onShow()
{
    d_ptr->sqlModel->setTable("students");
    d_ptr->sqlModel->select();
}

void MainWindow::setupUI()
{
    auto addButton = new QPushButton(tr("Add"), this);
    auto deleteButtom = new QPushButton(tr("Delete"), this);
    auto updateButton = new QPushButton(tr("Update"), this);
    auto selectButton = new QPushButton(tr("Select"), this);
    auto revertButton = new QPushButton(tr("Revert"), this);
    auto showAllButton = new QPushButton(tr("Show All"), this);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(deleteButtom, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::onUpdate);
    connect(selectButton, &QPushButton::clicked, this, &MainWindow::onSelect);
    connect(revertButton, &QPushButton::clicked, this, &MainWindow::onSelect);
    connect(showAllButton, &QPushButton::clicked, this, &MainWindow::onShow);

    auto buttonLayout = new QVBoxLayout;
    buttonLayout->setSpacing(0);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButtom);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(revertButton);
    buttonLayout->addWidget(showAllButton);

    auto bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(new QLabel(tr("Name: "), this));
    bottomLayout->addWidget(d_ptr->lineEdie);

    auto widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(widget);
    layout->addWidget(d_ptr->sqlTabView, 0, 0);
    layout->addLayout(buttonLayout, 0, 1);
    layout->addLayout(bottomLayout, 1, 0);
    layout->addWidget(selectButton, 1, 1);

    setCentralWidget(widget);
}

void MainWindow::setupModel()
{
    d_ptr->sqlModel = new QSqlTableModel(this);
    d_ptr->sqlModel->setTable("Students");
    d_ptr->sqlModel->select();
    // 设置编辑策略
    d_ptr->sqlModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    d_ptr->sqlTabView->setModel(d_ptr->sqlModel);
}

void MainWindow::showError(const QSqlError &err)
{
    QMessageBox::critical(this,
                          tr("Unable to initialize Database"),
                          tr("Error initializing database: ") + err.text());
}
