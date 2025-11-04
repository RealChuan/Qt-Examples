#include "interactiveimageitem.hpp"

#include <QDebug>
#include <QEvent>

class InteractiveImageItem::InteractiveImageItemPrivate
{
public:
    explicit InteractiveImageItemPrivate(InteractiveImageItem *q)
        : q_ptr(q)
    {}

    InteractiveImageItem *q_ptr;

    QPixmap sourcePixmap;
};

InteractiveImageItem::InteractiveImageItem(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent)
    , d_ptr(new InteractiveImageItemPrivate(this))
{
    setAcceptHoverEvents(true);
}

InteractiveImageItem::~InteractiveImageItem() {}

void InteractiveImageItem::setSourcePixmap(const QPixmap &pixmap)
{
    d_ptr->sourcePixmap = pixmap;
    setPixmap(pixmap);
}

QPixmap InteractiveImageItem::sourcePixmap() const
{
    return d_ptr->sourcePixmap;
}

void InteractiveImageItem::setSize(const QSize &size)
{
    if (!d_ptr->sourcePixmap.isNull()) {
        setPixmap(d_ptr->sourcePixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

QSize InteractiveImageItem::size() const
{
    return pixmap().size();
}

bool InteractiveImageItem::sceneEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
        event->accept();
        emit itemClicked();
        return true;
    case QEvent::GraphicsSceneHoverEnter: emit mouseEntered(); return true;
    case QEvent::GraphicsSceneHoverLeave: emit mouseLeft(); return true;
    default: break;
    }

    return QGraphicsPixmapItem::sceneEvent(event);
}
