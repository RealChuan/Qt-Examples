#include "gridmodel.h"

class GridModel::GridModelPrivate
{
public:
    explicit GridModelPrivate(GridModel *parent)
        : q_ptr(parent)
    {}

    GridModel *q_ptr;

    GridCellList cellList;
    int cellWidth = GRID_CELL_SIZE;
    int cellHeight = GRID_CELL_SIZE;
};

GridModel::GridModel(QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new GridModelPrivate(this))
{}

GridModel::~GridModel() = default;

int GridModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d_ptr->cellList.size();
}

QVariant GridModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= d_ptr->cellList.size()) {
        return {};
    }

    const auto &cell = d_ptr->cellList.at(index.row());

    switch (role) {
    case Qt::DecorationRole: return cell.image;
    case Qt::ToolTipRole: return cell.label;
    case Qt::SizeHintRole: return QSize{d_ptr->cellWidth, d_ptr->cellHeight};
    case Qt::TextAlignmentRole: return Qt::AlignCenter;
    default: break;
    }

    return {};
}

void GridModel::setCellList(const GridCellList &cellList)
{
    beginResetModel();
    d_ptr->cellList = cellList;
    endResetModel();
}

void GridModel::clearCells()
{
    beginResetModel();
    d_ptr->cellList.clear();
    endResetModel();
}

void GridModel::setCellSize(int width, int height)
{
    if (d_ptr->cellWidth != width || d_ptr->cellHeight != height) {
        d_ptr->cellWidth = width;
        d_ptr->cellHeight = height;
        emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, 0), {Qt::SizeHintRole});
    }
}

QSize GridModel::cellSize() const
{
    return QSize{d_ptr->cellWidth, d_ptr->cellHeight};
}
