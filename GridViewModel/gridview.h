#ifndef GRIDVIEW_H
#define GRIDVIEW_H

#include <QListView>

#include "gridmodel.h"

class GridView : public QListView
{
public:
    explicit GridView(QWidget *parent = nullptr);

    void setImageVector(const ImageVector &imageVector)
    {
        m_gridModel->setImageVector(imageVector);
    }

private slots:
    void onSelectChanged();

private:
    void setupUI();
    void buildConnect();

    GridModel *m_gridModel;
};

#endif // GRIDVIEW_H
