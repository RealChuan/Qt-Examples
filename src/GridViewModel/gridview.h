#pragma once

#include "gridmodel.h"

#include <QListView>

class GridView : public QListView
{
    Q_OBJECT

public:
    explicit GridView(QWidget *parent = nullptr);
    ~GridView() override;

    void setCellList(const GridCellList &cellList);
    void clearCells();

private slots:
    void onSelectionChanged();

private:
    void setupUI();
    void buildConnect();

    class GridViewPrivate;
    QScopedPointer<GridViewPrivate> d_ptr;
};
