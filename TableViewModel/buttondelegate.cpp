#include "buttondelegate.h"
#include "student.h"

#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QtWidgets>

ButtonDelegate::ButtonDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , m_buttonPtr(new QStyleOptionButton)
{
    
}

void ButtonDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    int w = qMin(option.rect.width(), option.rect.height()) / 10.0;
    m_buttonPtr->rect = option.rect.adjusted(w, w, -w, -w);
    m_buttonPtr->text = index.model()->data(index).toString();
    m_buttonPtr->state |= QStyle::State_Enabled;

    painter->save();

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setBrush(option.palette.highlightedText());
    }

    QPushButton button;
    qApp->style()->drawControl(QStyle::CE_PushButton, m_buttonPtr.data(), painter, &button);

    painter->restore();
}

bool ButtonDelegate::editorEvent(QEvent *event,
                                 QAbstractItemModel *model,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index)
{
    int w = qMin(option.rect.width(), option.rect.height()) / 10.0;

    switch (event->type()) {
    case QEvent::MouseButtonPress:{
        QMouseEvent* mouseEvent =(QMouseEvent*)event;
        if (option.rect.adjusted(w, w, -w, -w).contains(mouseEvent->pos())) {
            m_buttonPtr->state |= QStyle::State_Sunken;
        }
    } break;
    case QEvent::MouseButtonRelease:{
        QMouseEvent* mouseEvent =(QMouseEvent*)event;
        if (option.rect.adjusted(w, w, -w, -w).contains(mouseEvent->pos())) {
            m_buttonPtr->state &= (~QStyle::State_Sunken);

            Student stu = model->data(index, Qt::UserRole).value<Student>();
            QString details = tr("This Student id = %1, name = %2, age = %3, "
                                 "gender = %4, achievement = %5")
                                  .arg(stu.id())
                                  .arg(stu.name())
                                  .arg(stu.age())
                                  .arg(stu.gender())
                                  .arg(stu.achievement());
            QDialog dialog;
            QHBoxLayout *layout = new QHBoxLayout(&dialog);
            layout->addWidget(new QLabel(details, &dialog));
            dialog.exec();
        }
    }
    break;
    default: break;
    }
    return true;
}
