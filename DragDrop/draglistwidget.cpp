#include "draglistwidget.h"

DragListWidget::DragListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setDragEnabled(true);
    QStringList list{tr("Hello"), tr("World"), tr("Good"), tr("Study"), tr("Day"), tr("Up")};
    addItems(list);
}
