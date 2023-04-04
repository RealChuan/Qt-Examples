#ifndef GRIDMODEL_H
#define GRIDMODEL_H

#include <QAbstractListModel>
#include <QImage>

#define WIDTH 80

struct ImageInfo
{
    QImage image;
    QString color;
};

using ImageVector = QVector<ImageInfo *>;

class GridModel : public QAbstractListModel
{
public:
    explicit GridModel(QObject *parent = nullptr);
    [[nodiscard]] auto rowCount(const QModelIndex & = QModelIndex()) const -> int override
    {
        return m_imageVector.size();
    }
    [[nodiscard]] auto data(const QModelIndex &index, int role = Qt::DisplayRole) const
        -> QVariant override;

    void setImageVector(const ImageVector &imageVector)
    {
        beginResetModel();
        m_imageVector = imageVector;
        endResetModel();
    }

private:
    ImageVector m_imageVector;
};

#endif // GRIDMODEL_H
