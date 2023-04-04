#include "displaytablemodel.hpp"
#include "starrating.h"

#include <QApplication>
#include <QStyle>

DisplayTableModel::DisplayTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    auto metaEnums = QMetaEnum::fromType<Property>();
    for (int i = 0; i < metaEnums.keyCount(); ++i) {
        m_headerDatas << metaEnums.key(i);
    }
}

auto DisplayTableModel::data(const QModelIndex &index, int role) const -> QVariant
{
    if (!index.isValid()) {
        return {};
    }

    auto row = index.row();
    auto col = index.column();

    const auto &data = m_datas.at(row);
    switch (role) {
    case Qt::TextAlignmentRole: return Qt::AlignCenter;
    case Qt::CheckStateRole:
        if (ID == col) {
            return data.checked() ? Qt::Checked : Qt::Unchecked;
        }
        break;
    case Qt::DecorationRole:
        if (ID == col) {
            return qApp->style()->standardIcon(QStyle::SP_ComputerIcon);
        }
        break;
    case Qt::WhatsThisRole:
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
    case Qt::EditRole: { //双击为空需添加
        switch (col) {
        case ID: return row;
        case TITLE: return data.title();
        case NUMBER: return data.number();
        case STATE: return data.state();
        case PROCESS: return data.process();
        case RICHTEXT: return data.richText();
        case DETAILS: return tr("Details");
        default: break;
        }
        break;
    case Qt::UserRole:
        if (col == RATING) {
            return QVariant::fromValue(StarRating(data.rating()));
        }
        return QVariant::fromValue(data);
    }
    default: break;
    }
    return {};
}

auto DisplayTableModel::setData(const QModelIndex &index, const QVariant &value, int role) -> bool
{
    if (!index.isValid()) {
        return false;
    }

    auto row = index.row();
    auto col = index.column();

    auto data = m_datas.at(row);
    switch (role) {
    case Qt::CheckStateRole:
        if (ID == col) {
            data.setChecked(value.toBool());
        }
        break;
    case Qt::EditRole: {
        switch (col) {
        case TITLE: data.setTitle(value.toString()); break;
        case NUMBER: data.setNumber(value.toInt()); break;
        case STATE: data.setState(value.toString()); break;
        case PROCESS: data.setProcess(value.toInt()); break;
        case RATING: data.setRating(value.value<StarRating>().starCount()); break;
        case RICHTEXT: data.setRichText(value.toString()); break;
        default: break;
        }
    }
    default: break;
    }
    emit dataChanged(index, index);
    return true;
}

auto DisplayTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    -> QVariant
{
    if (section < 0 || section >= m_headerDatas.size() || orientation != Qt::Horizontal) {
        return {};
    }
    switch (role) {
    case Qt::TextAlignmentRole: return Qt::AlignCenter;
    case Qt::WhatsThisRole:
    case Qt::ToolTipRole:
    case Qt::DisplayRole: return m_headerDatas.at(section);
    default: break;
    }
    return {};
}

Qt::ItemFlags DisplayTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return {};
    }
    auto flags = QAbstractTableModel::flags(index);
    if (index.column() == ID) {
        flags |= Qt::ItemIsUserCheckable;
    } else {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}
