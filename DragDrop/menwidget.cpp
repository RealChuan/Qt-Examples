#include "menwidget.h"

#include <QDrag>

menWidget::menWidget(QWidget *parent)
    :QListWidget(parent)
{
    setMinimumWidth(100);
    setDragEnabled(true);
    QStringList menulist;
    menulist<<"111"<<"222"<<"333"<<"444"<<"555"<<"666"<<"777"<<"888"<<"999";
    addItems(menulist);
}
