#pragma once

#include <QGraphicsPixmapItem>
#include <QObject>

#include <memory>

class InteractiveImageItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(QSize size READ size WRITE setSize)

public:
    explicit InteractiveImageItem(QGraphicsItem *parent = nullptr);
    ~InteractiveImageItem() override;

    void setSourcePixmap(const QPixmap &pixmap);
    [[nodiscard]] QPixmap sourcePixmap() const;

    void setSize(const QSize &size);
    [[nodiscard]] QSize size() const;

signals:
    void itemClicked();
    void mouseEntered();
    void mouseLeft();

protected:
    bool sceneEvent(QEvent *event) override;

private:
    class InteractiveImageItemPrivate;
    std::unique_ptr<InteractiveImageItemPrivate> d_ptr;
};
