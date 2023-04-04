#pragma once

#include "displaydata.hpp"

#include <QAbstractTableModel>

class DisplayTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Property { ID, TITLE, NUMBER, STATE, PROCESS, RATING, DETAILS, RICHTEXT };
    Q_ENUM(Property);

    explicit DisplayTableModel(QObject *parent = nullptr);

    [[nodiscard]] auto rowCount(const QModelIndex & = QModelIndex()) const -> int
    {
        return m_datas.size();
    }
    [[nodiscard]] auto columnCount(const QModelIndex & = QModelIndex()) const -> int
    {
        return m_headerDatas.size();
    }

    [[nodiscard]] auto data(const QModelIndex &index, int role = Qt::DisplayRole) const -> QVariant;
    auto setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) -> bool;
    [[nodiscard]] auto headerData(int section,
                                  Qt::Orientation orientation,
                                  int role = Qt::DisplayRole) const -> QVariant;
    [[nodiscard]] auto flags(const QModelIndex &index) const -> Qt::ItemFlags;

    void setDatas(const QVector<DisplayInfo> &datas)
    {
        beginResetModel();
        m_datas = datas;
        endResetModel();
    }

private:
    QVector<DisplayInfo> m_datas;
    QStringList m_headerDatas;
};
