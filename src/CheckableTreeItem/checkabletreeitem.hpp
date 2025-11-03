#pragma once

#include <QStandardItem>
#include <QVariant>

/**
 * @brief 支持层次化复选框状态联动的树形项
 * 
 * 该类扩展了QStandardItem，提供了父子节点间复选框状态的自动联动更新功能。
 * 当父节点状态改变时，会自动更新所有子节点状态；当子节点状态改变时，会自动更新父节点状态。
 * 支持三态：选中、未选中、部分选中。
 */
class CheckableTreeItem : public QStandardItem
{
public:
    /**
     * @brief 构造函数
     * @param text 项的显示文本
     */
    explicit CheckableTreeItem(const QString &text);

    /**
     * @brief 设置项数据
     * @param value 要设置的数据值
     * @param role 数据角色，默认为Qt::UserRole + 1
     * 
     * 重写setData方法以处理复选框状态的联动更新。
     */
    void setData(const QVariant &value, int role = Qt::UserRole + 1) override;

    /**
     * @brief 获取当前复选框状态
     * @return 当前的复选框状态
     */
    Qt::CheckState checkState() const
    {
        return static_cast<Qt::CheckState>(data(Qt::CheckStateRole).toInt());
    }

private:
    /**
     * @brief 更新所有子节点的复选框状态
     * @param state 要设置的状态
     */
    void updateChildrenState(Qt::CheckState state);

    /**
     * @brief 更新父节点的复选框状态
     */
    void updateParentState();

    /**
     * @brief 根据子节点状态计算当前节点的应有状态
     * @return 计算出的复选框状态
     */
    Qt::CheckState calculateStateFromChildren() const;

    /// 防止递归更新的标志
    bool m_updating = false;
};
