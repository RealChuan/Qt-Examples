#ifndef LISTVIEW_HPP
#define LISTVIEW_HPP

#include "treeview.hpp"

#include <QFileInfo>

class ListView : public TreeView
{
    Q_OBJECT
public:
    explicit ListView(QWidget *parent = nullptr);
    ~ListView() override;

    void setDatas(const QFileInfoList &fileInfos);

private slots:
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    void setupUI();
    void buildConnect();
    void updateCheckState();

    class ListViewPrivate;
    QScopedPointer<ListViewPrivate> d_ptr;
};

#endif // LISTVIEW_HPP
