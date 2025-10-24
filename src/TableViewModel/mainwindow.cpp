#include "mainwindow.h"
#include "displaytableview.hpp"

#include <QtWidgets>

class MainWindow::MainWindowPrivate
{
public:
    explicit MainWindowPrivate(MainWindow *q)
        : q_ptr(q)
    {
        tableView = new DisplayTableView(q_ptr);
    }

    MainWindow *q_ptr;

    DisplayTableView *tableView;
    DisplayInfoList datas;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new MainWindowPrivate(this))
{
    setupUI();
    init();
    resize(1000, 618);
}

MainWindow::~MainWindow() = default;

void MainWindow::onInsertItem()
{
    int row = d_ptr->tableView->currentIndex().row();

    auto data = DisplayInfo{};
    if (row < 0 || row >= d_ptr->datas.size()) {
        d_ptr->datas.append(data);
    } else {
        d_ptr->datas.insert(row, data);
    }

    d_ptr->tableView->setDatas(d_ptr->datas);
}

void MainWindow::onRemoveItem()
{
    auto index = d_ptr->tableView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    int row = index.row();
    d_ptr->datas.takeAt(row);
    d_ptr->tableView->setDatas(d_ptr->datas);
}

void MainWindow::init()
{
    d_ptr->datas.append({true,
                         tr("Book"),
                         static_cast<qint16>(QRandomGenerator::global()->bounded(100)),
                         tr("close"),
                         50,
                         3,
                         "R<span style='color:#F25643;'>RichText1</span>xxxxxxxxxxxxx"});
    d_ptr->datas.append(
        {false, tr("Pen"), 100, tr("open"), 20, 2, "R<span style='color:#F25643;'>RichText2</span>x"});
    d_ptr->datas.append({true,
                         tr("Pencil"),
                         200,
                         tr("close"),
                         30,
                         1,
                         "R<span style='color:#F25643;'>RichText3</span>x"});
    d_ptr->datas.append({false,
                         tr("Eraser"),
                         static_cast<qint16>(QRandomGenerator::global()->bounded(100)),
                         tr("open"),
                         40,
                         4,
                         "R<span style='color:#F25643;'>RichText4</span>yyyyyyyyyyyy"});
    d_ptr->datas.append({true,
                         tr("Ruler"),
                         500,
                         tr("close"),
                         50,
                         5,
                         "R<span style='color:#F25643;'>RichText5</span>x"});

    while (d_ptr->datas.size() < 1000 * 100) {
        d_ptr->datas.append(d_ptr->datas);
    }

    d_ptr->tableView->setDatas(d_ptr->datas);
    d_ptr->tableView->selectRow(d_ptr->datas.size() - 1);
}

void MainWindow::setupUI()
{
    auto *addBtn = new QPushButton(tr("Add"), this);
    auto *removeBtn = new QPushButton(tr("Remove"), this);

    auto *hLayout = new QHBoxLayout;
    hLayout->addStretch(1);
    hLayout->addWidget(addBtn);
    hLayout->addWidget(removeBtn);

    auto *widget = new QWidget(this);
    auto *layout = new QVBoxLayout(widget);
    layout->addLayout(hLayout);
    layout->addWidget(d_ptr->tableView);
    setCentralWidget(widget);

    connect(d_ptr->tableView, &DisplayTableView::insertItem, this, &MainWindow::onInsertItem);
    connect(d_ptr->tableView, &DisplayTableView::removeItem, this, &MainWindow::onRemoveItem);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onInsertItem);
    connect(removeBtn, &QPushButton::clicked, this, &MainWindow::onRemoveItem);
}
