#include "carousel3dview.hpp"
#include "interactiveimageitem.hpp"

#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QTimer>

namespace {

struct CarouselLayout
{
    QRectF geometry;
    int zOrder = 1;
    qreal opacity = 1.0;
};

using CarouselLayoutList = QList<CarouselLayout>;

class CarouselElement
{
public:
    CarouselElement(InteractiveImageItem *item,
                    QPropertyAnimation *posAnimation,
                    QPropertyAnimation *sizeAnimation)
        : imageItemPtr(item)
        , posAnimationPtr(posAnimation)
        , sizeAnimationPtr(sizeAnimation)
    {}

    ~CarouselElement() = default;

    QScopedPointer<InteractiveImageItem> imageItemPtr;
    QScopedPointer<QPropertyAnimation> posAnimationPtr;
    QScopedPointer<QPropertyAnimation> sizeAnimationPtr;
};

using CarouselElementList = QList<QSharedPointer<CarouselElement>>;

QPixmap createPlaceholderPixmap(const QString &text, const QSize &size)
{
    const QRect rect(QPoint(0, 0), size);

    QImage image(size, QImage::Format_RGBA8888_Premultiplied);
    image.fill(Qt::black);
    QPainter painter(&image);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    painter.setOpacity(0.5);
    QFont font;
    font.setPixelSize(qMin(48, size.height() / 6));
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(rect, text, QTextOption(Qt::AlignCenter));
    painter.end();

    return QPixmap::fromImage(image);
}

} // namespace

class Carousel3DView::Carousel3DViewPrivate
{
public:
    explicit Carousel3DViewPrivate(Carousel3DView *q)
        : q_ptr(q)
        , animationDuration(800)
        , autoRotationInterval(2000)
        , autoRotationEnabled(true)
    {
        initializeScene();
        initializeLayouts();
        initializeElements();
        startAutoRotation();
    }

    ~Carousel3DViewPrivate() = default;

    void initializeScene()
    {
        // 使用固定的场景尺寸
        q_ptr->setScene(new QGraphicsScene(QRect(0, 0, 600, 300), q_ptr));

        animationGroup = new QParallelAnimationGroup(q_ptr);
        autoRotationTimer = new QTimer(q_ptr);
        q_ptr->connect(autoRotationTimer, &QTimer::timeout, q_ptr, &Carousel3DView::rotateToNext);
    }

    void initializeLayouts()
    {
        // 基于固定场景尺寸计算布局
        QSizeF sceneSize(q_ptr->sceneRect().size());

        // 定义7个位置的相对比例
        struct LayoutRatio
        {
            qreal x, y, width, height;
            int zOrder;
            qreal opacity;
        };

        // 基于800x400的比例计算，保持与原始布局相似的比例
        QList<LayoutRatio> ratios = {
            {0.22, 0.28, 0.15, 0.375, 1, 0.4},   // 左3
            {0.075, 0.25, 0.162, 0.425, 2, 0.6}, // 左2
            {0.212, 0.19, 0.212, 0.545, 3, 0.8}, // 左1
            {0.4, 0.1, 0.28, 0.72, 4, 1.0},      // 中心
            {0.66, 0.19, 0.212, 0.545, 3, 0.8},  // 右1
            {0.85, 0.25, 0.162, 0.425, 2, 0.6},  // 右2
            {0.695, 0.28, 0.15, 0.375, 1, 0.4}   // 右3
        };

        for (const auto &ratio : ratios) {
            QRectF rect(sceneSize.width() * ratio.x,
                        sceneSize.height() * ratio.y,
                        sceneSize.width() * ratio.width,
                        sceneSize.height() * ratio.height);
            layoutConfigs << CarouselLayout{rect, ratio.zOrder, ratio.opacity};
        }
    }

    void initializeElements()
    {
        for (int i = 0; i < layoutConfigs.size(); ++i) {
            const auto &layout = layoutConfigs[i];

            auto *imageItem = new InteractiveImageItem;
            setupElementConnections(imageItem);
            setupElementAppearance(imageItem, layout, i);

            q_ptr->scene()->addItem(imageItem);

            auto *posAnimation = createPositionAnimation(imageItem);
            auto *sizeAnimation = createSizeAnimation(imageItem);

            animationGroup->addAnimation(posAnimation);
            animationGroup->addAnimation(sizeAnimation);

            auto element = QSharedPointer<CarouselElement>::create(imageItem,
                                                                   posAnimation,
                                                                   sizeAnimation);
            carouselElements.append(element);
        }
    }

    void setupElementConnections(InteractiveImageItem *item)
    {
        q_ptr->connect(item,
                       &InteractiveImageItem::itemClicked,
                       q_ptr,
                       &Carousel3DView::onItemClicked);
        q_ptr->connect(item, &InteractiveImageItem::mouseEntered, q_ptr, [this]() {
            if (autoRotationEnabled) {
                autoRotationTimer->stop();
            }
        });
        q_ptr->connect(item, &InteractiveImageItem::mouseLeft, q_ptr, [this]() {
            if (autoRotationEnabled) {
                autoRotationTimer->start();
            }
        });
    }

    void setupElementAppearance(InteractiveImageItem *item, const CarouselLayout &layout, int index)
    {
        item->setOpacity(layout.opacity);
        item->setPos(layout.geometry.topLeft());
        item->setZValue(layout.zOrder);

        // 使用固定尺寸创建占位图
        QSize itemSize = layout.geometry.size().toSize();
        auto pixmap = createPlaceholderPixmap(QString("Item %1").arg(index + 1), itemSize);
        item->setSourcePixmap(pixmap);
        item->setPixmap(pixmap);
    }

    QPropertyAnimation *createPositionAnimation(InteractiveImageItem *item)
    {
        auto *animation = new QPropertyAnimation(item, "pos", q_ptr);
        animation->setDuration(animationDuration);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        return animation;
    }

    QPropertyAnimation *createSizeAnimation(InteractiveImageItem *item)
    {
        auto *animation = new QPropertyAnimation(item, "size", q_ptr);
        animation->setDuration(animationDuration);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        return animation;
    }

    void updateAnimationDurations()
    {
        for (auto &element : carouselElements) {
            element->posAnimationPtr->setDuration(animationDuration);
            element->sizeAnimationPtr->setDuration(animationDuration);
        }
    }

    void startAutoRotation()
    {
        if (autoRotationEnabled) {
            autoRotationTimer->start(autoRotationInterval);
        }
    }

    void stopAutoRotation() { autoRotationTimer->stop(); }

    void resetLayout()
    {
        stopAutoRotation();

        Q_ASSERT(layoutConfigs.size() == carouselElements.size());

        for (int i = 0; i < layoutConfigs.size(); ++i) {
            const auto &targetLayout = layoutConfigs[i];
            auto element = carouselElements[i];

            element->imageItemPtr->setOpacity(targetLayout.opacity);
            element->imageItemPtr->setZValue(targetLayout.zOrder);

            element->posAnimationPtr->setStartValue(element->imageItemPtr->pos());
            element->posAnimationPtr->setEndValue(targetLayout.geometry.topLeft());

            element->sizeAnimationPtr->setStartValue(element->imageItemPtr->size());
            element->sizeAnimationPtr->setEndValue(targetLayout.geometry.size().toSize());
        }

        animationGroup->start();
        startAutoRotation();
    }

    int findElementIndex(InteractiveImageItem *item) const
    {
        for (int i = 0; i < carouselElements.size(); ++i) {
            if (carouselElements[i]->imageItemPtr.data() == item) {
                return i;
            }
        }
        return -1;
    }

    void jumpToIndex(int targetIndex)
    {
        if (targetIndex < 0 || targetIndex >= carouselElements.size()) {
            return;
        }

        int currentCenterIndex = carouselElements.size() / 2;
        int diff = targetIndex - currentCenterIndex;

        if (diff > 0) {
            for (int i = 0; i < diff; ++i) {
                q_ptr->rotateToNext();
            }
        } else if (diff < 0) {
            for (int i = 0; i < -diff; ++i) {
                q_ptr->rotateToPrevious();
            }
        }
    }

    Carousel3DView *q_ptr;

    CarouselLayoutList layoutConfigs;
    CarouselElementList carouselElements;
    QParallelAnimationGroup *animationGroup;
    QTimer *autoRotationTimer;

    // 新增配置参数
    int animationDuration;
    int autoRotationInterval;
    bool autoRotationEnabled;
};

Carousel3DView::Carousel3DView(QWidget *parent)
    : QGraphicsView(parent)
    , d_ptr(new Carousel3DViewPrivate(this))
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setFrameStyle(QFrame::NoFrame);

    // 设置滚动条策略为不显示，因为我们使用fitInView来适应大小
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

Carousel3DView::~Carousel3DView() {}

// 动画持续时间设置
void Carousel3DView::setAnimationDuration(int duration)
{
    if (duration > 0 && duration != d_ptr->animationDuration) {
        d_ptr->animationDuration = duration;
        d_ptr->updateAnimationDurations();
    }
}

int Carousel3DView::animationDuration() const
{
    return d_ptr->animationDuration;
}

// 自动轮播间隔设置
void Carousel3DView::setAutoRotationInterval(int interval)
{
    if (interval > 0 && interval != d_ptr->autoRotationInterval) {
        d_ptr->autoRotationInterval = interval;
        if (d_ptr->autoRotationEnabled && d_ptr->autoRotationTimer->isActive()) {
            d_ptr->autoRotationTimer->start(interval);
        }
    }
}

int Carousel3DView::autoRotationInterval() const
{
    return d_ptr->autoRotationInterval;
}

// 自动轮播启用设置
void Carousel3DView::setAutoRotationEnabled(bool enabled)
{
    if (d_ptr->autoRotationEnabled != enabled) {
        d_ptr->autoRotationEnabled = enabled;
        if (enabled) {
            d_ptr->startAutoRotation();
        } else {
            d_ptr->stopAutoRotation();
        }
    }
}

bool Carousel3DView::isAutoRotationEnabled() const
{
    return d_ptr->autoRotationEnabled;
}

// 自动轮播控制
void Carousel3DView::startAutoRotation()
{
    setAutoRotationEnabled(true);
}

void Carousel3DView::stopAutoRotation()
{
    setAutoRotationEnabled(false);
}

void Carousel3DView::toggleAutoRotation()
{
    setAutoRotationEnabled(!d_ptr->autoRotationEnabled);
}

void Carousel3DView::rotateToNext()
{
    if (!d_ptr->carouselElements.isEmpty()) {
        auto element = d_ptr->carouselElements.takeFirst();
        d_ptr->carouselElements.append(element);
        d_ptr->resetLayout();
    }
}

void Carousel3DView::rotateToPrevious()
{
    if (!d_ptr->carouselElements.isEmpty()) {
        auto element = d_ptr->carouselElements.takeLast();
        d_ptr->carouselElements.prepend(element);
        d_ptr->resetLayout();
    }
}

void Carousel3DView::onItemClicked()
{
    auto *clickedItem = qobject_cast<InteractiveImageItem *>(sender());
    if (!clickedItem) {
        return;
    }

    int clickedIndex = d_ptr->findElementIndex(clickedItem);
    if (clickedIndex == -1) {
        return;
    }

    d_ptr->jumpToIndex(clickedIndex);
}

void Carousel3DView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    // 只使用视图缩放功能，不改变场景或图片
    fitInView(sceneRect(), Qt::KeepAspectRatio);
}

void Carousel3DView::showEvent(QShowEvent *event)
{
    QGraphicsView::showEvent(event);

    // 确保首次显示时正确适应视图
    if (scene()) {
        fitInView(sceneRect(), Qt::KeepAspectRatio);
    }
}
