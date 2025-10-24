#include "gridview.h"

#include <QDebug>

GridView::GridView(QWidget *parent)
    : QListView(parent)
{
    setupUI();
    buildConnect();
}

void GridView::onSelectChanged()
{
    QItemSelectionModel *selModel = selectionModel();
    if (!selModel) {
        return;
    }
    qDebug() << selModel->selectedIndexes();
}

void GridView::setupUI()
{
    setFocusPolicy(Qt::NoFocus);
    setFlow(LeftToRight);
    setResizeMode(Adjust);
    setViewMode(IconMode);
    setSelectionMode(ExtendedSelection);
    setSpacing(20);

    m_gridModel = new GridModel(this);
    setModel(m_gridModel);
}

void GridView::buildConnect()
{
    connect(selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &GridView::onSelectChanged);
}
