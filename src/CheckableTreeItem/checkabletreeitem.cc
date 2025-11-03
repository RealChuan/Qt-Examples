#include "checkabletreeitem.hpp"

#include <QDebug>

CheckableTreeItem::CheckableTreeItem(const QString &text)
    : QStandardItem(text)
{
    setCheckable(true);
    // 初始状态设置为未选中
    setData(Qt::Unchecked, Qt::CheckStateRole);
}

void CheckableTreeItem::setData(const QVariant &value, int role)
{
    // 只处理复选框状态角色的变化
    if (role == Qt::CheckStateRole) {
        // 检查值类型
        if (!value.canConvert<int>()) {
            qWarning() << "Invalid check state value type";
            QStandardItem::setData(value, role);
            return;
        }

        Qt::CheckState newState = static_cast<Qt::CheckState>(value.toInt());
        Qt::CheckState currentState = checkState();

        // 如果状态没有变化或者正在更新中，直接返回避免递归
        if (newState == currentState || m_updating) {
            QStandardItem::setData(value, role);
            return;
        }

        // 设置更新标志，防止递归
        m_updating = true;

        switch (newState) {
        case Qt::Unchecked:
        case Qt::Checked:
            // 对于明确的选中/未选中状态，更新所有子节点
            updateChildrenState(newState);
            // 更新当前节点状态
            QStandardItem::setData(newState, role);
            // 通知父节点更新状态
            updateParentState();
            break;

        case Qt::PartiallyChecked:
            // 对于部分选中状态，基于子节点计算实际应该设置的状态
            Qt::CheckState actualState = calculateStateFromChildren();
            if (actualState != currentState) {
                QStandardItem::setData(actualState, role);
                updateParentState();
            } else {
                // 如果状态没有变化，仍然需要设置数据以确保一致性
                QStandardItem::setData(actualState, role);
            }
            break;
        }

        // 清除更新标志
        m_updating = false;
        return;
    }

    // 对于其他角色，直接调用基类实现
    QStandardItem::setData(value, role);
}

void CheckableTreeItem::updateChildrenState(Qt::CheckState state)
{
    for (int i = 0; i < rowCount(); i++) {
        QStandardItem *childItem = child(i);
        if (childItem && childItem->isCheckable()) {
            // 递归更新子节点状态
            childItem->setData(state, Qt::CheckStateRole);
        }
    }
}

void CheckableTreeItem::updateParentState()
{
    QStandardItem *parentItem = parent();
    if (parentItem && parentItem->isCheckable()) {
        // 动态转换以调用正确的calculateStateFromChildren方法
        CheckableTreeItem *treeParent = dynamic_cast<CheckableTreeItem *>(parentItem);
        if (treeParent) {
            Qt::CheckState parentNewState = treeParent->calculateStateFromChildren();
            parentItem->setData(parentNewState, Qt::CheckStateRole);
        } else {
            // 如果父节点不是CheckableTreeItem，使用默认逻辑
            int checkedCount = 0;
            int uncheckedCount = 0;
            int totalCheckable = 0;

            for (int i = 0; i < parentItem->rowCount(); i++) {
                QStandardItem *child = parentItem->child(i);
                if (child && child->isCheckable()) {
                    totalCheckable++;
                    Qt::CheckState childState = static_cast<Qt::CheckState>(
                        child->data(Qt::CheckStateRole).toInt());
                    if (childState == Qt::Checked) {
                        checkedCount++;
                    } else if (childState == Qt::Unchecked) {
                        uncheckedCount++;
                    }
                }
            }

            Qt::CheckState newState;
            if (totalCheckable == 0) {
                newState = parentItem->checkState(); // 保持原状态
            } else if (checkedCount == totalCheckable) {
                newState = Qt::Checked;
            } else if (uncheckedCount == totalCheckable) {
                newState = Qt::Unchecked;
            } else {
                newState = Qt::PartiallyChecked;
            }

            parentItem->setData(newState, Qt::CheckStateRole);
        }
    }
}

Qt::CheckState CheckableTreeItem::calculateStateFromChildren() const
{
    // 如果没有子节点，返回当前状态
    if (rowCount() == 0) {
        return checkState();
    }

    int checkedCount = 0;
    int uncheckedCount = 0;
    int totalCheckable = 0;

    // 统计子节点状态
    for (int i = 0; i < rowCount(); i++) {
        QStandardItem *childItem = child(i);
        if (childItem && childItem->isCheckable()) {
            totalCheckable++;
            Qt::CheckState childState = static_cast<Qt::CheckState>(
                childItem->data(Qt::CheckStateRole).toInt());
            switch (childState) {
            case Qt::Checked: checkedCount++; break;
            case Qt::Unchecked: uncheckedCount++; break;
            case Qt::PartiallyChecked:
                // 如果任何子节点是部分选中状态，当前节点也应该是部分选中
                return Qt::PartiallyChecked;
            }
        }
    }

    // 根据统计结果确定状态
    if (totalCheckable == 0) {
        return checkState();
    } else if (checkedCount == totalCheckable) {
        return Qt::Checked;
    } else if (uncheckedCount == totalCheckable) {
        return Qt::Unchecked;
    } else {
        return Qt::PartiallyChecked;
    }
}