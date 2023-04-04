#include "mainwindow.h"
#include "displaytableview.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    init();
    resize(1000, 618);
}

MainWindow::~MainWindow() = default;

void MainWindow::onInsertItem()
{
    int row = m_table->currentIndex().row();

    auto data = DisplayInfo{};
    if (row < 0 || row >= m_datas.size()) {
        m_datas.append(data);
    } else {
        m_datas.insert(row, data);
    }

    m_table->setDatas(m_datas);
}

void MainWindow::onRemoveItem()
{
    QModelIndex index = m_table->currentIndex();
    if (!index.isValid()) {
        return;
    }
    int row = index.row();
    m_datas.takeAt(row);
    m_table->setDatas(m_datas);
}

void MainWindow::init()
{
    m_datas.append({true,
                    tr("Book"),
                    static_cast<qint16>(QRandomGenerator::global()->bounded(100)),
                    tr("close"),
                    50,
                    3,
                    "R<span style='color:#F25643;'>RichText1</span>xxxxxxxxxxxxx"});
    m_datas.append(
        {false, tr("Pen"), 100, tr("open"), 20, 2, "R<span style='color:#F25643;'>RichText2</span>x"});
    m_datas.append({true,
                    tr("Pencil"),
                    200,
                    tr("close"),
                    30,
                    1,
                    "R<span style='color:#F25643;'>RichText3</span>x"});
    m_datas.append({false,
                    tr("Eraser"),
                    static_cast<qint16>(QRandomGenerator::global()->bounded(100)),
                    tr("open"),
                    40,
                    4,
                    "R<span style='color:#F25643;'>RichText4</span>yyyyyyyyyyyy"});
    m_datas.append({true,
                    tr("Ruler"),
                    500,
                    tr("close"),
                    50,
                    5,
                    "R<span style='color:#F25643;'>RichText5</span>x"});

    while (m_datas.size() < 1000 * 100) {
        m_datas.append(m_datas);
    }

    m_table->setDatas(m_datas);
    m_table->selectRow(m_datas.size() - 1);
}

void MainWindow::setupUI()
{
    auto addBtn = new QPushButton(tr("Add"), this);
    auto removeBtn = new QPushButton(tr("Remove"), this);
    m_table = new DisplayTableView(this);

    auto hLayout = new QHBoxLayout;
    hLayout->addStretch(1);
    hLayout->addWidget(addBtn);
    hLayout->addWidget(removeBtn);

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->addLayout(hLayout);
    layout->addWidget(m_table);
    setCentralWidget(widget);

    connect(m_table, &DisplayTableView::insertItem, this, &MainWindow::onInsertItem);
    connect(m_table, &DisplayTableView::removeItem, this, &MainWindow::onRemoveItem);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onInsertItem);
    connect(removeBtn, &QPushButton::clicked, this, &MainWindow::onRemoveItem);
}
