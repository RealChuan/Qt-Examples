#include "gridview.h"

class GridView::GridViewPrivate
{
public:
    explicit GridViewPrivate(GridView *parent)
        : q_ptr(parent)
        , gridModel(new GridModel(parent))
    {}

    GridView *q_ptr;

    GridModel *gridModel;
};

GridView::GridView(QWidget *parent)
    : QListView(parent)
    , d_ptr(new GridViewPrivate(this))
{
    setupUI();
    buildConnect();
}

GridView::~GridView() = default;

void GridView::setCellList(const GridCellList &cellList)
{
    d_ptr->gridModel->setCellList(cellList);
}

void GridView::clearCells()
{
    d_ptr->gridModel->clearCells();
}

void GridView::onSelectionChanged()
{
    QItemSelectionModel *selModel = selectionModel();
    if (!selModel) {
        return;
    }

    const auto selectedIndexes = selModel->selectedIndexes();
    for (const auto &index : selectedIndexes) {
        if (index.isValid()) {
            auto label = index.data(Qt::ToolTipRole).toString();
            // 可以在这里处理选中项
        }
    }
}

void GridView::setupUI()
{
    setFocusPolicy(Qt::NoFocus);
    setFlow(LeftToRight);
    setResizeMode(Adjust);
    setViewMode(IconMode);
    setSelectionMode(ExtendedSelection);
    setSpacing(20);
    // setUniformItemSizes(true);
    // setWrapping(true);

    setModel(d_ptr->gridModel);
}

void GridView::buildConnect()
{
    connect(selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &GridView::onSelectionChanged);
}
