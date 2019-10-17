#include "acceptwidget.h"

#include <QMenu>

acceptWidget::acceptWidget(QWidget *parent)
    :QListWidget(parent)
{
    setMinimumWidth(200);
    setDragEnabled(true);
    setAcceptDrops(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    ListMenuButton=new QMenu(this);
    ListMenuButton->addAction("删除",[this]
    {
        removeItemWidget(currentItem());
        delete currentItem();
    });

    connect(this,&QWidget::customContextMenuRequested,[this]
    {
        if (!currentItem())
            return;
        ListMenuButton->exec(QCursor::pos());
    });

}
