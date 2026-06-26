#pragma once

#include "gridmodel.hpp"

#include <QListView>

#include <memory>

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
    std::unique_ptr<GridViewPrivate> d_ptr;
};
