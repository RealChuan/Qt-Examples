#include "imagecarousel.hpp"
#include "graphicspixmapitem.hpp"

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QTimer>

struct ItemData
{
    QRectF rect;
    int z = 1;
    qreal opacity = 1.0;
};

using ItemDataList = QList<ItemData>;

struct PixmapItem
{
    PixmapItem(GraphicsPixmapItem *item,
               QPropertyAnimation *posAnimation,
               QPropertyAnimation *sizeAnimation)
        : graphicsPixmapItemPtr(item)
        , posAnimationPtr(posAnimation)
        , sizeAnimationPtr(sizeAnimation)
    {}
    ~PixmapItem() {}

    QScopedPointer<GraphicsPixmapItem> graphicsPixmapItemPtr;
    QScopedPointer<QPropertyAnimation> posAnimationPtr;
    QScopedPointer<QPropertyAnimation> sizeAnimationPtr;
};

using PixmapItemList = QList<PixmapItem *>;

QPixmap createPixmap(const QString &text, const QSize &size)
{
    const QRect rect(QPoint(0, 0), size);

    QImage image(size, QImage::Format_RGBA8888_Premultiplied);
    image.fill(Qt::black);
    QPainter painter(&image);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    painter.setOpacity(0.5);
    QFont font;
    font.setPixelSize(64);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(rect, text, QTextOption(Qt::AlignCenter));
    painter.end();

    return QPixmap::fromImage(image);
}

class ImageCarousel::ImageViewerPrivate
{
public:
    explicit ImageViewerPrivate(ImageCarousel *q)
        : q_ptr(q)
    {
        q_ptr->setScene(new QGraphicsScene(QRect(0, 0, 876, 368), q_ptr));

        animationGroup = new QParallelAnimationGroup(q_ptr);
        timer = new QTimer(q_ptr);
        q_ptr->connect(timer, &QTimer::timeout, q_ptr, &ImageCarousel::onNext);
        timer->setInterval(1500);

        initList();
        timer->start();
    }

    ~ImageViewerPrivate() { pixmapItemList.clear(); }

    void initList()
    {
        itemDataList << ItemData{QRectF(194, 111, 120, 150), 1, 0.4}
                     << ItemData{QRectF(60, 101, 130, 170), 2, 0.6}
                     << ItemData{QRectF(170, 77, 170, 218), 3, 0.8}
                     << ItemData{QRectF(322, 40, 224, 288), 4, 1}
                     << ItemData{QRectF(528, 77, 170, 218), 3, 0.8}
                     << ItemData{QRectF(680, 101, 130, 170), 2, 0.6}
                     << ItemData{QRectF(556, 111, 120, 150), 1, 0.4};

        for (const auto &itemData : std::as_const(itemDataList)) {
            auto *graphicsPixmapItem = new GraphicsPixmapItem;
            q_ptr->connect(graphicsPixmapItem,
                           &GraphicsPixmapItem::clicked,
                           q_ptr,
                           &ImageCarousel::onClicked);
            q_ptr->connect(graphicsPixmapItem, &GraphicsPixmapItem::enter, timer, &QTimer::stop);
            q_ptr->connect(graphicsPixmapItem, &GraphicsPixmapItem::leave, timer, [this]() {
                timer->start();
            });
            graphicsPixmapItem->setOpacity(itemData.opacity);
            graphicsPixmapItem->setPos(itemData.rect.topLeft());
            graphicsPixmapItem->setZValue(itemData.z);
            auto pixmap = createPixmap(QString::number(itemData.z), {224, 288});
            graphicsPixmapItem->setCustomPixmap(pixmap);
            graphicsPixmapItem->setPixmap(pixmap.scaled(itemData.rect.size().toSize(),
                                                        Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation));

            q_ptr->scene()->addItem(graphicsPixmapItem);

            auto *posAnimation = new QPropertyAnimation(graphicsPixmapItem, "pos", q_ptr);
            posAnimation->setDuration(1000);
            posAnimation->setEasingCurve(QEasingCurve::OutQuad);
            animationGroup->addAnimation(posAnimation);

            auto *sizeAnimation = new QPropertyAnimation(graphicsPixmapItem, "size", q_ptr);
            sizeAnimation->setDuration(1000);
            sizeAnimation->setEasingCurve(QEasingCurve::OutQuad);
            animationGroup->addAnimation(sizeAnimation);

            pixmapItemList.append(new PixmapItem{graphicsPixmapItem, posAnimation, sizeAnimation});
        }
    }

    void reset()
    {
        timer->stop();
        Q_ASSERT(itemDataList.size() == pixmapItemList.size());
        for (int i = 0; i < itemDataList.size(); ++i) {
            auto itemData = itemDataList[i];
            auto pixmapItem = pixmapItemList[i];

            pixmapItem->graphicsPixmapItemPtr->setOpacity(itemData.opacity);
            pixmapItem->graphicsPixmapItemPtr->setZValue(itemData.z);
            // pixmapItem->graphicsPixmapItemPtr->setSize(itemData.rect.size().toSize());
            // pixmapItem->itemPtr->setPos(itemData.rect.topLeft());
            pixmapItem->posAnimationPtr->setStartValue(pixmapItem->graphicsPixmapItemPtr->pos());
            pixmapItem->posAnimationPtr->setEndValue(itemData.rect.topLeft());
            pixmapItem->sizeAnimationPtr->setStartValue(pixmapItem->graphicsPixmapItemPtr->size());
            pixmapItem->sizeAnimationPtr->setEndValue(itemData.rect.size().toSize());
        }

        animationGroup->start();
        timer->start();
    }

    ImageCarousel *q_ptr;

    ItemDataList itemDataList;
    PixmapItemList pixmapItemList;
    QParallelAnimationGroup *animationGroup;

    QTimer *timer;
};

ImageCarousel::ImageCarousel(QWidget *parent)
    : QGraphicsView(parent)
    , d_ptr(new ImageViewerPrivate(this))
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

ImageCarousel::~ImageCarousel() {}

void ImageCarousel::onPrevious()
{
    auto *item = d_ptr->pixmapItemList.takeLast();
    d_ptr->pixmapItemList.prepend(item);
    d_ptr->reset();
}

void ImageCarousel::onNext()
{
    auto *item = d_ptr->pixmapItemList.takeFirst();
    d_ptr->pixmapItemList.append(item);
    d_ptr->reset();
}

void ImageCarousel::onClicked()
{
    auto *graphicsPixmapItem = qobject_cast<GraphicsPixmapItem *>(sender());
    if (!graphicsPixmapItem) {
        return;
    }

    int index = 0;
    for (int i = 0; i < d_ptr->pixmapItemList.size(); ++i) {
        if (d_ptr->pixmapItemList.at(i)->graphicsPixmapItemPtr.data() == graphicsPixmapItem) {
            index = i;
            break;
        }
    }
    auto diff = index - d_ptr->pixmapItemList.size() / 2;
    qDebug() << index << diff;
    if (diff > 0) {
        while (diff-- != 0) {
            onNext();
        }
    } else if (diff < 0) {
        while (diff++ != 0) {
            onPrevious();
        }
    }
}

void ImageCarousel::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fitInView(sceneRect(), Qt::KeepAspectRatio);
}
