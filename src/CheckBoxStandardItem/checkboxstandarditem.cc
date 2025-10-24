#include "checkboxstandarditem.hpp"

CheckBoxStandardItem::CheckBoxStandardItem(const QString &text)
    : QStandardItem(text)
{
    setCheckable(true);
}

void CheckBoxStandardItem::setData(const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole) { //针对复选框变动做操作
        Qt::CheckState check_state = (Qt::CheckState) value.toInt();
        switch (check_state) {
        case Qt::Unchecked: //取消
            for (int i = 0, num = rowCount(); i < num; i++) {
                child(i)->setData(Qt::Unchecked, Qt::CheckStateRole);
            }
            //修改内容-必须先修改自己再通知父节点
            QStandardItem::setData(value, role);
            //通知父节点，我取消了选择，直接告诉父节点半选即可
            if (parent()) {
                parent()->setData(Qt::PartiallyChecked, role);
            }
            return;                                      //此事件已完成直接return
        case Qt::PartiallyChecked: {                     //半选
            Qt::CheckState current_state = checkState(); //当前状态
            int checked_num = 0;                         //被选择的数量
            int unchecked_num = 0;                       //未选择的数量
            bool is_partially = false;
            Qt::CheckState child_state;
            int m_rowCount = rowCount();
            //遍历所有子节点
            for (int i = 0; i < m_rowCount; i++) {
                child_state = child(i)->checkState();
                //子节点半选，则直接半选
                switch (child_state) {
                case Qt::PartiallyChecked: is_partially = true; break;
                case Qt::Unchecked: unchecked_num++; break;
                case Qt::Checked: checked_num++; break;
                default: checked_num++; break;
                }
            }
            //根据子节点状态确定当前节点应该设置的状态
            Qt::CheckState now_state;
            if (is_partially) {
                now_state = Qt::PartiallyChecked;
            } else if (checked_num == m_rowCount) {
                now_state = Qt::Checked;
            } else if (unchecked_num == m_rowCount) {
                now_state = Qt::Unchecked;
            } else {
                now_state = Qt::PartiallyChecked;
            }
            //修改状态并通知父节点
            if (current_state != now_state) {
                //修改内容-必须先修改自己再通知父节点
                QStandardItem::setData(now_state, role);
                //通知父节点，我的状态更改,也就是父节点进入半选
                if (parent()) {
                    parent()->setData(Qt::PartiallyChecked, role);
                }
            }
        }
            return;       //此事件已完成直接return
        case Qt::Checked: //全选
            for (int i = 0, num = rowCount(); i < num; i++) {
                child(i)->setData(Qt::Checked, Qt::CheckStateRole);
            }
            //修改内容-必须先修改自己再通知父节点
            QStandardItem::setData(value, role);
            //通知父节点，我被选了,也就是父节点进入半选
            if (parent()) {
                parent()->setData(Qt::PartiallyChecked, role);
            }
            return; //此事件已完成直接return
        default:    //如果出现此情况就是错了，可以加错误处理
            break;
        }
    }
    QStandardItem::setData(value, role);
}
