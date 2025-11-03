#include "bubblewrapperwidget.hpp"

#include <QApplication>
#include <QEventLoop>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QScreen>

class BubbleWrapperWidget::BubbleWrapperWidgetPrivate
{
public:
    explicit BubbleWrapperWidgetPrivate(BubbleWrapperWidget *q)
        : q_ptr(q)
    {
        bubble = new BubbleWidget(q_ptr);

        auto *shadowEffect = new QGraphicsDropShadowEffect(bubble);
        shadowEffect->setOffset(0, 0);
        shadowEffect->setColor(Qt::gray);
        shadowEffect->setBlurRadius(margin);
        bubble->setGraphicsEffect(shadowEffect);
    }

    ~BubbleWrapperWidgetPrivate() = default;

    // 主对象指针
    BubbleWrapperWidget *q_ptr;

    // 内部气泡控件
    BubbleWidget *bubble = nullptr;

    // 效果和动画
    QPropertyAnimation *showAnimation = nullptr;
    QPropertyAnimation *hideAnimation = nullptr;
    // 可配置属性
    int animationDuration = 300;

    // 状态标志
    bool isAnimating = false;
    bool isShowing = false;

    const int margin = 10;
};

BubbleWrapperWidget::BubbleWrapperWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new BubbleWrapperWidgetPrivate(this))
{
    setupUI();
    buildConnect();

    QMetaObject::invokeMethod(this, &BubbleWrapperWidget::onEnsureInitialized, Qt::QueuedConnection);
}

BubbleWrapperWidget::~BubbleWrapperWidget() = default;

// 获取内部气泡控件
BubbleWidget *BubbleWrapperWidget::bubbleWidget() const
{
    return d_ptr->bubble;
}

auto BubbleWrapperWidget::sizeHint() const -> QSize
{
    auto size = d_ptr->bubble->sizeHint();
    return {size.width() + d_ptr->margin * 2, size.height() + d_ptr->margin * 2};
}

auto BubbleWrapperWidget::minimumSizeHint() const -> QSize
{
    auto size = d_ptr->bubble->minimumSizeHint();
    return {size.width() + d_ptr->margin * 2, size.height() + d_ptr->margin * 2};
}

// 动画设置
void BubbleWrapperWidget::setAnimationDuration(int duration)
{
    if (d_ptr->animationDuration != duration) {
        d_ptr->animationDuration = qMax(0, duration);
        d_ptr->showAnimation->setDuration(d_ptr->animationDuration);
        d_ptr->hideAnimation->setDuration(d_ptr->animationDuration);
    }
}

auto BubbleWrapperWidget::animationDuration() const -> int
{
    return d_ptr->animationDuration;
}

bool BubbleWrapperWidget::isAnimating() const
{
    return d_ptr->isAnimating;
}

// 显示控制
void BubbleWrapperWidget::showAt(const QPoint &pos, BubbleWidget::Direction direction)
{
    if (!d_ptr->bubble) {
        return;
    }

    d_ptr->bubble->setDirection(direction);

    QSize size = this->size();
    QPoint adjustedPos = pos;
    const QRect screenGeometry = getAvailableScreenGeometry();

    // 获取三角形大小
    QSize triSize = d_ptr->bubble->triangleSize();

    // 根据三角形顶点精确调整位置
    switch (direction) {
    case BubbleWidget::Direction::Top:
        // 三角形顶点在底部中心，指向下方目标位置
        adjustedPos -= QPoint(size.width() / 2, triSize.height() + size.height());
        break;
    case BubbleWidget::Direction::Bottom:
        // 三角形顶点在顶部中心，指向上方目标位置
        adjustedPos -= QPoint(size.width() / 2, -triSize.height());
        break;
    case BubbleWidget::Direction::Left:
        // 三角形顶点在右侧中心，指向右方目标位置
        adjustedPos -= QPoint(triSize.width() + size.width(), size.height() / 2);
        break;
    case BubbleWidget::Direction::Right:
        // 三角形顶点在左侧中心，指向左方目标位置
        adjustedPos -= QPoint(-triSize.width(), size.height() / 2);
        break;
    }

    // 确保在屏幕边界内
    adjustedPos.setX(
        qMax(screenGeometry.left(), qMin(adjustedPos.x(), screenGeometry.right() - size.width())));
    adjustedPos.setY(
        qMax(screenGeometry.top(), qMin(adjustedPos.y(), screenGeometry.bottom() - size.height())));

    move(adjustedPos);
    showAnimated();
}

void BubbleWrapperWidget::showNearWidget(QWidget *widget,
                                         BubbleWidget::Direction direction,
                                         int margin)
{
    if (!widget)
        return;

    QRect widgetRect = widget->geometry();
    QPoint targetPos;

    switch (direction) {
    case BubbleWidget::Direction::Top:
        targetPos = widget->mapToGlobal(QPoint(widgetRect.width() / 2, -margin));
        break;
    case BubbleWidget::Direction::Bottom:
        targetPos = widget->mapToGlobal(
            QPoint(widgetRect.width() / 2, widgetRect.height() + margin));
        break;
    case BubbleWidget::Direction::Left:
        targetPos = widget->mapToGlobal(QPoint(-margin, widgetRect.height() / 2));
        break;
    case BubbleWidget::Direction::Right:
        targetPos = widget->mapToGlobal(
            QPoint(widgetRect.width() + margin, widgetRect.height() / 2));
        break;
    }

    showAt(targetPos, direction);
}

// 动画控制
void BubbleWrapperWidget::showAnimated()
{
    if (d_ptr->animationDuration <= 0) {
        show();
        return;
    }

    if (d_ptr->isAnimating) {
        stopAnimation();
    }

    startShowAnimation();
}

void BubbleWrapperWidget::hideAnimated()
{
    if (d_ptr->animationDuration <= 0) {
        close();
        return;
    }

    if (d_ptr->isAnimating) {
        stopAnimation();
    }

    startHideAnimation();
}

void BubbleWrapperWidget::toggleAnimation()
{
    if (d_ptr->isAnimating) {
        stopAnimation();
    } else if (isVisible()) {
        hideAnimated();
    } else {
        showAnimated();
    }
}

void BubbleWrapperWidget::stopAnimation()
{
    if (d_ptr->showAnimation->state() == QPropertyAnimation::Running) {
        d_ptr->showAnimation->stop();
    }
    if (d_ptr->hideAnimation->state() == QPropertyAnimation::Running) {
        d_ptr->hideAnimation->stop();
    }
    d_ptr->isAnimating = false;
    emit animationStopped();
}

// 事件处理
void BubbleWrapperWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToClose();
    QWidget::closeEvent(event);
}

void BubbleWrapperWidget::mousePressEvent(QMouseEvent *event)
{
    hideAnimated();
    QWidget::mousePressEvent(event);
}

void BubbleWrapperWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideAnimated();
    }
    QWidget::keyPressEvent(event);
}

void BubbleWrapperWidget::showEvent(QShowEvent *event)
{
    emit aboutToShow();
    QWidget::showEvent(event);
}

void BubbleWrapperWidget::setupUI()
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Popup | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(d_ptr->margin, d_ptr->margin, d_ptr->margin, d_ptr->margin);
    layout->addWidget(d_ptr->bubble);
}

void BubbleWrapperWidget::buildConnect()
{
    // 初始化显示动画
    d_ptr->showAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    d_ptr->showAnimation->setEasingCurve(QEasingCurve::OutCubic);
    d_ptr->showAnimation->setDuration(d_ptr->animationDuration);
    d_ptr->showAnimation->setStartValue(0.0);
    d_ptr->showAnimation->setEndValue(1.0);

    // 初始化隐藏动画
    d_ptr->hideAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    d_ptr->hideAnimation->setEasingCurve(QEasingCurve::InCubic);
    d_ptr->hideAnimation->setDuration(d_ptr->animationDuration);
    d_ptr->hideAnimation->setStartValue(1.0);
    d_ptr->hideAnimation->setEndValue(0.0);

    // 连接动画信号
    connect(d_ptr->showAnimation,
            &QPropertyAnimation::finished,
            this,
            &BubbleWrapperWidget::onShowAnimationFinished);
    connect(d_ptr->hideAnimation,
            &QPropertyAnimation::finished,
            this,
            &BubbleWrapperWidget::onHideAnimationFinished);
}

auto BubbleWrapperWidget::getAvailableScreenGeometry() const -> QRect
{
    QWidget *parent = parentWidget();
    if (parent) {
        QScreen *screen = parent->screen();
        if (screen) {
            return screen->availableGeometry();
        }
    }

    // 回退到桌面可用区域
    return QApplication::primaryScreen()->availableGeometry();
}

void BubbleWrapperWidget::startShowAnimation()
{
    d_ptr->isAnimating = true;
    d_ptr->isShowing = true;
    setWindowOpacity(0.0);
    show();

    d_ptr->showAnimation->start();
    emit animationStarted();
}

void BubbleWrapperWidget::startHideAnimation()
{
    d_ptr->isAnimating = true;
    d_ptr->isShowing = false;

    d_ptr->hideAnimation->start();
    emit animationStarted();
}

void BubbleWrapperWidget::onEnsureInitialized()
{
    if (!isVisible()) {
        updateGeometry();
        adjustSize();
    }
}

void BubbleWrapperWidget::onShowAnimationFinished()
{
    d_ptr->isAnimating = false;
    emit animationFinished();
}

void BubbleWrapperWidget::onHideAnimationFinished()
{
    d_ptr->isAnimating = false;
    close();
    emit animationFinished();
}
