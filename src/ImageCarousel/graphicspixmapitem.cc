#include "graphicspixmapitem.hpp"

#include <QEvent>

class GraphicsPixmapItem::GraphicsPixmapItemPrivate
{
public:
    GraphicsPixmapItemPrivate(GraphicsPixmapItem *q)
        : q_ptr(q)
    {}

    GraphicsPixmapItem *q_ptr;

    QPixmap pixmap;
};

GraphicsPixmapItem::GraphicsPixmapItem(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent)
    , d_ptr(new GraphicsPixmapItemPrivate(this))
{
    setAcceptHoverEvents(true);
}

GraphicsPixmapItem::~GraphicsPixmapItem() {}

void GraphicsPixmapItem::setCustomPixmap(const QPixmap &pixmap)
{
    d_ptr->pixmap = pixmap;
}

QPixmap GraphicsPixmapItem::customPixmap() const
{
    return d_ptr->pixmap;
}

void GraphicsPixmapItem::setSize(const QSize &size)
{
    setPixmap(d_ptr->pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QSize GraphicsPixmapItem::size() const
{
    return pixmap().size();
}

bool GraphicsPixmapItem::sceneEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
        event->accept();
        clicked();
        return true;
    case QEvent::GraphicsSceneHoverEnter:
        qDebug() << "HoverEnter";
        emit enter();
        return true;
    case QEvent::GraphicsSceneHoverLeave:
        qDebug() << "HoverLeave";
        emit leave();
        return true;
    default: break;
    }

    return QGraphicsPixmapItem::sceneEvent(event);
}
