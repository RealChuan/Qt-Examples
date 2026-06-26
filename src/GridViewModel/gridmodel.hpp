#pragma once

#include <QAbstractListModel>
#include <QImage>
#include <QList>

#include <memory>

struct GridCell
{
    QImage image;
    QString label;
};

using GridCellList = QList<GridCell>;

class GridModel : public QAbstractListModel
{
    Q_OBJECT

public:
    [[maybe_unused]] static constexpr int DEFAULT_GRID_CELL_SIZE = 80;

    explicit GridModel(QObject *parent = nullptr);
    ~GridModel() override;

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index,
                                int role = Qt::DisplayRole) const override;

    void setCellList(const GridCellList &cellList);
    void clearCells();

    void setCellSize(int width, int height);
    [[nodiscard]] QSize cellSize() const;

private:
    class GridModelPrivate;
    std::unique_ptr<GridModelPrivate> d_ptr;
};
