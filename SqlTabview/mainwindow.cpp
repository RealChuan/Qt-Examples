#include "mainwindow.h"
#include "sql.h"

#include <QtSql>
#include <QtWidgets>
#include <QMessageBox>

class MainWindowPrivate{
public:
    explicit MainWindowPrivate(QWidget *parent)
        :owner(parent){
        sqlTabView = new QTableView(owner);
        sqlTabView->setAlternatingRowColors(true);
        sqlTabView->horizontalHeader()->setStretchLastSection(true);
        lineEdie = new QLineEdit(owner);
    }
    QWidget *owner;
    Sql *sqlite;
    QSqlTableModel *sqlModel;
    QTableView *sqlTabView;
    QLineEdit *lineEdie;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d(new MainWindowPrivate(this))
{
    if(!Sql::searchSQLite())
        return;
    d->sqlite = new Sql(this);
    QSqlError error = d->sqlite->createOrOpenSqlite();
    if(error.type() != QSqlError::NoError){
        showError(error);
        return;
    }
    setupUI();
    setupModel();
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::onAdd()
{
    // 获得表的行数
    int rowNum = d->sqlModel->rowCount();
    QSqlQuery query;
    int id = query.lastInsertId().toInt();
    // 添加一行
    d->sqlModel->insertRow(rowNum);
    d->sqlModel->setData(d->sqlModel->index(rowNum,0),id+1);
}

void MainWindow::onDelete()
{
    int curRow = d->sqlTabView->currentIndex().row();
    d->sqlModel->removeRow(curRow);
    int ok = QMessageBox::warning(this,tr("Delete current row!"),
                                  tr("Are you sure you want to delete the current line?"),
                                  QMessageBox::Yes, QMessageBox::No);
    if(ok == QMessageBox::No)
        d->sqlModel->revertAll();// 如果不删除， 则撤销
    else
        d->sqlModel->submitAll();// 否则提交， 在数据库中删除该行
}

void MainWindow::onUpdate()
{
    // 开始事务操作
    d->sqlModel->database().transaction();
    if (d->sqlModel->submitAll())
        d->sqlModel->database().commit(); //提交
    else
    {
        d->sqlModel->database().rollback(); //回滚
        QMessageBox::warning(this, tr("tableModel"),
                             tr("Database error: %1").
                             arg(d->sqlModel->lastError().text()));
    }
}

void MainWindow::onSelect()
{
    QString name = d->lineEdie->text().trimmed();
    if(name.isEmpty())
        QMessageBox::warning(this, tr("Warning"), tr("Name cannot be empty!") );
    else{
        //根据姓名进行筛选， 一定要使用单引号
        d->sqlModel->setFilter(QString("Name = '%1'").arg(name));
        d->sqlModel->select();
    }
}

void MainWindow::onRevert()
{
    d->sqlModel->revertAll();
}

void MainWindow::onShow()
{
    d->sqlModel->setTable("students");
    d->sqlModel->select();
}

void MainWindow::setupUI()
{
    QPushButton *addButton = new QPushButton(tr("Add"), this);
    QPushButton *deleteButtom = new QPushButton(tr("Delete"), this);
    QPushButton *updateButton = new QPushButton(tr("Update"), this);
    QPushButton *selectButton = new QPushButton(tr("Select"), this);
    QPushButton *revertButton = new QPushButton(tr("Revert"), this);
    QPushButton *showAllButton = new QPushButton(tr("Show All"), this);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(deleteButtom, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::onUpdate);
    connect(selectButton, &QPushButton::clicked, this, &MainWindow::onSelect);
    connect(revertButton, &QPushButton::clicked, this, &MainWindow::onSelect);
    connect(showAllButton, &QPushButton::clicked, this, &MainWindow::onShow);

    QVBoxLayout *buttonLayout = new QVBoxLayout(this);
    buttonLayout->setSpacing(0);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButtom);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(revertButton);
    buttonLayout->addWidget(showAllButton);

    QHBoxLayout *bottomLayout = new QHBoxLayout(this);
    bottomLayout->addWidget(new QLabel(tr("Name: "), this));
    bottomLayout->addWidget(d->lineEdie);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(d->sqlTabView, 0, 0);
    layout->addLayout(buttonLayout, 0, 1);
    layout->addLayout(bottomLayout, 1, 0);
    layout->addWidget(selectButton, 1, 1);

    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);
    setCentralWidget(frame);
    setMinimumSize(640, 480);
}

void MainWindow::setupModel()
{
    d->sqlModel = new QSqlTableModel(this);
    d->sqlModel->setTable("Students");
    d->sqlModel->select();
    // 设置编辑策略
    d->sqlModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    d->sqlTabView->setModel(d->sqlModel);
}

void MainWindow::showError(const QSqlError &err)
{
    QMessageBox::critical(this, tr("Unable to initialize Database"),
                          tr("Error initializing database: ") + err.text());

}

