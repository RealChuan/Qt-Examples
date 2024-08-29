#pragma once

#include <QGraphicsPixmapItem>

class GraphicsPixmapItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(QSize size READ size WRITE setSize)
public:
    explicit GraphicsPixmapItem(QGraphicsItem *parent = nullptr);
    ~GraphicsPixmapItem() override;

    void setCustomPixmap(const QPixmap &pixmap);
    QPixmap customPixmap() const;

    void setSize(const QSize &size);
    QSize size() const;

signals:
    void clicked();
    void enter();
    void leave();

protected:
    bool sceneEvent(QEvent *event) override;

private:
    class GraphicsPixmapItemPrivate;
    QScopedPointer<GraphicsPixmapItemPrivate> d_ptr;
};
