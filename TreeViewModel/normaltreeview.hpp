#ifndef NORMALTREEVIEW_HPP
#define NORMALTREEVIEW_HPP

#include "treemodel.hpp"
#include "treeview.hpp"

#include <QFileInfo>

class NormalTreeModel : public BaseTreeModel
{
    Q_OBJECT
public:
    explicit NormalTreeModel(QObject *parent);

    void setTr();

    void setDatas(const QFileInfoList &fileInfos, bool depth = false);

    int rowCount(const QModelIndex &idx = QModelIndex()) const override;

protected:
    int columnCount(const QModelIndex &idx) const override { return m_headerList.size(); }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QStringList m_headerList;
};

class NormalTreeView : public TreeView
{
    Q_OBJECT
public:
    explicit NormalTreeView(QWidget *parent = nullptr);
    ~NormalTreeView();

    void setDatas(const QFileInfoList &fileInfos);

private:
    void setupUI();

    class NormalTreeViewPrivate;
    QScopedPointer<NormalTreeViewPrivate> d_ptr;
};

#endif // NORMALTREEVIEW_HPP
