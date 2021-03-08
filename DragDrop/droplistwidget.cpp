#include "droplistwidget.h"

#include <QMenu>

DropListWidget::DropListWidget(QWidget *parent)
    :QListWidget(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setupUI();
}

void DropListWidget::setupUI()
{
    menu = new QMenu(this);
    menu->addAction(tr("delete"), this, [this]{
        removeItemWidget(currentItem());
        delete currentItem();
    });
    connect(this, &DropListWidget::customContextMenuRequested, this, [this]{
        if (!currentItem())
            return;
        menu->exec(QCursor::pos());
    });
}
