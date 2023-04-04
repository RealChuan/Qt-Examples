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

    [[nodiscard]] auto rowCount(const QModelIndex &idx = QModelIndex()) const -> int override;

protected:
    [[nodiscard]] auto columnCount(const QModelIndex &idx) const -> int override
    {
        return m_headerList.size();
    }
    [[nodiscard]] auto headerData(int section, Qt::Orientation orientation, int role) const
        -> QVariant override;

private:
    QStringList m_headerList;
};

class NormalTreeView : public TreeView
{
    Q_OBJECT
public:
    explicit NormalTreeView(QWidget *parent = nullptr);
    ~NormalTreeView() override;

    void setDatas(const QFileInfoList &fileInfos);

private:
    void setupUI();

    class NormalTreeViewPrivate;
    QScopedPointer<NormalTreeViewPrivate> d_ptr;
};

#endif // NORMALTREEVIEW_HPP
