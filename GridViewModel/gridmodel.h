#ifndef GRIDMODEL_H
#define GRIDMODEL_H

#include <QAbstractListModel>
#include <QImage>

#define WIDTH 80

struct ImageInfo{
    QImage image;
    QString color;
};

typedef QVector<ImageInfo*> ImageVector;

class GridModel : public QAbstractListModel
{
public:
    GridModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex & = QModelIndex()) const override
    { return m_imageVector.size(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setImageVector(const ImageVector& imageVector)
    {
        beginResetModel();
        m_imageVector = imageVector;
        endResetModel();
    }
private:
    ImageVector m_imageVector;
};

#endif // GRIDMODEL_H
