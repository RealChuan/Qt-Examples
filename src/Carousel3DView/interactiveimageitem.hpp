#pragma once

#include <QGraphicsPixmapItem>
#include <QObject>

class InteractiveImageItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(QSize size READ size WRITE setSize)

public:
    explicit InteractiveImageItem(QGraphicsItem *parent = nullptr);
    ~InteractiveImageItem() override;

    void setSourcePixmap(const QPixmap &pixmap);
    QPixmap sourcePixmap() const;

    void setSize(const QSize &size);
    QSize size() const;

signals:
    void itemClicked();
    void mouseEntered();
    void mouseLeft();

protected:
    bool sceneEvent(QEvent *event) override;

private:
    class InteractiveImageItemPrivate;
    QScopedPointer<InteractiveImageItemPrivate> d_ptr;
};
