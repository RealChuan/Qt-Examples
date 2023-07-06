#include "imagecarousel.hpp"

#include <QGraphicsPixmapItem>
#include <QTimer>

class ImageCarousel::ImageCarouselPrivate
{
public:
    explicit ImageCarouselPrivate(ImageCarousel *q)
        : q_ptr(q)
    {
        timer = new QTimer(q);
        timer->setSingleShot(false);
    }

    ImageCarousel *q_ptr;

    QTimer *timer;
    int msec = 1000;
    int index = 0;
    QList<QGraphicsPixmapItem *> items;
};

ImageCarousel::ImageCarousel(QWidget *parent)
    : QGraphicsView(parent)
    , d_ptr(new ImageCarouselPrivate(this))
{
    setScene(new QGraphicsScene(this));
    setViewportUpdateMode(SmartViewportUpdate);
    setFrameShape(QFrame::NoFrame);
    setRenderHint(QPainter::SmoothPixmapTransform);

    connect(d_ptr->timer, &QTimer::timeout, this, &ImageCarousel::next);
}

ImageCarousel::~ImageCarousel() = default;

auto ImageCarousel::addImage(const QString &filename) -> bool
{
    auto pixmap = QPixmap(filename);
    if (pixmap.isNull()) {
        return false;
    }

    auto *item = new QGraphicsPixmapItem(pixmap);
    item->setVisible(false);
    scene()->addItem(item);
    d_ptr->items.append(item);

    if (d_ptr->items.count() == 1) {
        item->setVisible(true);
    }

    return true;
}

auto ImageCarousel::addImages(const QStringList &filenames) -> int
{
    int count = 0;
    for (const auto &filename : qAsConst(filenames)) {
        if (addImage(filename)) {
            ++count;
        }
    }
    return count;
}

void ImageCarousel::clearImages()
{
    for (auto item : qAsConst(d_ptr->items)) {
        scene()->removeItem(item);
        delete item;
    }
    d_ptr->items.clear();
}

auto ImageCarousel::imageCount() const -> int
{
    return d_ptr->items.count();
}

void ImageCarousel::start()
{
    d_ptr->timer->start(d_ptr->msec);
}

void ImageCarousel::stop()
{
    d_ptr->timer->stop();
}

void ImageCarousel::setInterval(int msec)
{
    d_ptr->msec = msec;
}

auto ImageCarousel::interval() const -> int
{
    return d_ptr->msec;
}

void ImageCarousel::onImageChanged(int index)
{
    Q_ASSERT(index >= 0 && index < d_ptr->items.count());

    d_ptr->items[d_ptr->index]->setVisible(false);
    d_ptr->index = index;
    d_ptr->items[d_ptr->index]->setVisible(true);

    resetItem();
}

void ImageCarousel::next()
{
    if (d_ptr->items.isEmpty()) {
        return;
    }

    d_ptr->items[d_ptr->index]->setVisible(false);
    d_ptr->index = (d_ptr->index + 1) % d_ptr->items.count();
    d_ptr->items[d_ptr->index]->setVisible(true);

    emit imageChanged(d_ptr->index);

    resetItem();
}

void ImageCarousel::resetItem()
{
    auto item = d_ptr->items[d_ptr->index];
    scene()->setSceneRect(item->boundingRect());
    resetTransform();
    auto size = item->pixmap().size();
    if (size.width() > width() || size.height() > height()) {
        fitInView(item, Qt::KeepAspectRatio);
    }
}
