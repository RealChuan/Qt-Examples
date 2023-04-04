#include "gridmodel.h"

#include <QDebug>

GridModel::GridModel(QObject *parent)
    : QAbstractListModel(parent)
{}

auto GridModel::data(const QModelIndex &index, int role) const -> QVariant
{
    ImageInfo *image = m_imageVector.at(index.row());

    switch (role) {
    case Qt::DecorationRole: return image->image;
    //case Qt::DisplayRole: return image->filename;
    case Qt::ToolTipRole: return image->color;
    case Qt::SizeHintRole: return QSize(WIDTH, WIDTH);
    case Qt::TextAlignmentRole: return Qt::AlignCenter;
    default: break;
    }

    return {};
}
