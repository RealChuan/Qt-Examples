#include "switchbutton.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>

class SwitchButton::SwitchButtonPrivate
{
public:
    explicit SwitchButtonPrivate(SwitchButton *q)
        : q_ptr(q)
    {}

    SwitchButton *q_ptr;

    // 可配置属性
    QColor checkedColor = QColor(77, 161, 255);
    QColor uncheckedColor = QColor(220, 220, 220);
    QColor thumbColor = Qt::white;
    QColor thumbBorderColor = QColor(180, 180, 180);
    int animationDuration = 120;

    // 状态标志
    bool hover = false;
    bool animating = false;

    // 动画
    QPropertyAnimation *animation = nullptr;
    int offset = 0;

    // 常量定义
    static constexpr double WIDTH_MARGIN_RATIO = 1.0 / 22.0;
    static constexpr double HEIGHT_MARGIN_RATIO = 1.0 / 22.0;
};

SwitchButton::SwitchButton(QWidget *parent)
    : QAbstractButton(parent)
    , d_ptr(new SwitchButtonPrivate(this))
{
    d_ptr->animation = new QPropertyAnimation(this, "offset", this);
    d_ptr->animation->setDuration(d_ptr->animationDuration);
    d_ptr->animation->setEasingCurve(QEasingCurve::InOutQuad);

    setCheckable(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // 连接信号
    connect(this, &SwitchButton::toggled, this, &SwitchButton::onToggled);
    connect(d_ptr->animation,
            &QPropertyAnimation::finished,
            this,
            &SwitchButton::onAnimationFinished);

    // 初始位置设置
    updateThumbPosition();
}

SwitchButton::~SwitchButton() = default;

auto SwitchButton::minimumSizeHint() const -> QSize
{
    return {50, 30};
}

// Checked color
void SwitchButton::setCheckedColor(const QColor &color)
{
    if (d_ptr->checkedColor != color) {
        d_ptr->checkedColor = color;
        update();
        emit checkedColorChanged(color);
    }
}

auto SwitchButton::checkedColor() const -> QColor
{
    return d_ptr->checkedColor;
}

// Unchecked color
void SwitchButton::setUncheckedColor(const QColor &color)
{
    if (d_ptr->uncheckedColor != color) {
        d_ptr->uncheckedColor = color;
        update();
        emit uncheckedColorChanged(color);
    }
}

auto SwitchButton::uncheckedColor() const -> QColor
{
    return d_ptr->uncheckedColor;
}

// Thumb color
void SwitchButton::setThumbColor(const QColor &color)
{
    if (d_ptr->thumbColor != color) {
        d_ptr->thumbColor = color;
        update();
        emit thumbColorChanged(color);
    }
}

auto SwitchButton::thumbColor() const -> QColor
{
    return d_ptr->thumbColor;
}

// Thumb border color
void SwitchButton::setThumbBorderColor(const QColor &color)
{
    if (d_ptr->thumbBorderColor != color) {
        d_ptr->thumbBorderColor = color;
        update();
        emit thumbBorderColorChanged(color);
    }
}

auto SwitchButton::thumbBorderColor() const -> QColor
{
    return d_ptr->thumbBorderColor;
}

// Animation duration
void SwitchButton::setAnimationDuration(int duration)
{
    if (duration >= 0 && d_ptr->animationDuration != duration) {
        d_ptr->animationDuration = duration;
        d_ptr->animation->setDuration(duration);
        emit animationDurationChanged(duration);
    }
}

auto SwitchButton::animationDuration() const -> int
{
    return d_ptr->animationDuration;
}

bool SwitchButton::isAnimating() const
{
    return d_ptr->animating;
}

// Offset property
auto SwitchButton::offset() const -> int
{
    return d_ptr->offset;
}

void SwitchButton::setOffset(int offset)
{
    if (d_ptr->offset != offset) {
        d_ptr->offset = offset;
        update();
        emit offsetChanged(offset);
    }
}

// Painting
void SwitchButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    drawBackground(&painter, slotRect());

    // 绘制滑块
    drawThumb(&painter, thumbRect());
}

void SwitchButton::enterEvent(QEnterEvent *event)
{
    QAbstractButton::enterEvent(event);
    setCursor(Qt::PointingHandCursor);
    d_ptr->hover = true;
    update();
}

void SwitchButton::leaveEvent(QEvent *event)
{
    QAbstractButton::leaveEvent(event);
    d_ptr->hover = false;
    update();
}

void SwitchButton::resizeEvent(QResizeEvent *event)
{
    QAbstractButton::resizeEvent(event);
    updateThumbPosition();
}

// Private slots
void SwitchButton::onToggled(bool checked)
{
    startAnimation(checked);
}

void SwitchButton::onAnimationFinished()
{
    d_ptr->animating = false;
    emit animationFinished(isChecked());
}

// Private methods
void SwitchButton::updateThumbPosition()
{
    if (!d_ptr->animating) {
        const double pos = isChecked() ? (width() - widthMargin() - thumbSize()) : widthMargin();
        setOffset(static_cast<int>(pos));
    }
}

void SwitchButton::startAnimation(bool checked)
{
    if (d_ptr->animation->state() == QPropertyAnimation::Running) {
        d_ptr->animation->stop();
    }

    // 预计算常用值
    const double widthMarginValue = widthMargin();
    const double thumbSizeValue = thumbSize();

    const double start = d_ptr->offset;
    const double end = checked ? (width() - widthMarginValue - thumbSizeValue) : widthMarginValue;

    d_ptr->animation->setStartValue(static_cast<int>(start));
    d_ptr->animation->setEndValue(static_cast<int>(end));

    d_ptr->animating = true;
    emit animationStarted(checked);

    d_ptr->animation->start();
}

void SwitchButton::drawBackground(QPainter *painter, const QRectF &rect)
{
    const double roundness = rect.height() / 2;

    // 预计算常用值
    const double thumbSizeValue = thumbSize();
    const double widthMarginValue = widthMargin();

    // 计算滑块中心位置相对于整个滑动范围的比例
    const double minPos = widthMarginValue;
    const double maxPos = width() - widthMarginValue - thumbSizeValue;
    double fillRatio = 0.0;

    if (maxPos > minPos) {
        fillRatio = (d_ptr->offset - minPos) / (maxPos - minPos);
        fillRatio = qBound(0.0, fillRatio, 1.0);
    }

    // 如果不在动画中，根据checked状态确定填充比例
    if (!d_ptr->animating) {
        fillRatio = isChecked() ? 1.0 : 0.0;
    }

    // 绘制未选中状态的背景
    painter->setBrush(d_ptr->uncheckedColor);
    painter->drawRoundedRect(rect, roundness, roundness);

    // 绘制选中状态的填充
    if (fillRatio > 0) {
        // 计算填充宽度，最小为thumbSize，最大为整个背景宽度
        const double minFillWidth = thumbSizeValue;
        const double maxFillWidth = rect.width();
        const double fillWidth = minFillWidth + fillRatio * (maxFillWidth - minFillWidth);

        QRectF fillRect = rect;
        fillRect.setWidth(fillWidth);

        painter->setBrush(d_ptr->checkedColor);
        painter->drawRoundedRect(fillRect, roundness, roundness);
    }
}

void SwitchButton::drawThumb(QPainter *painter, const QRectF &rect)
{
    // 绘制滑块边框
    QColor borderColor = d_ptr->thumbBorderColor;
    if (d_ptr->hover) {
        borderColor = isChecked() ? d_ptr->checkedColor : d_ptr->thumbBorderColor.darker(120);
    }

    painter->setBrush(borderColor);
    painter->drawEllipse(rect);

    // 绘制滑块主体
    QRectF innerRect = rect.adjusted(2, 2, -2, -2);
    painter->setBrush(d_ptr->thumbColor);
    painter->drawEllipse(innerRect);
}

// Geometry calculations
auto SwitchButton::widthMargin() const -> double
{
    return width() * SwitchButtonPrivate::WIDTH_MARGIN_RATIO;
}

auto SwitchButton::heightMargin() const -> double
{
    return height() * SwitchButtonPrivate::HEIGHT_MARGIN_RATIO;
}

auto SwitchButton::thumbSize() const -> double
{
    return height() - 2 * heightMargin();
}

auto SwitchButton::slotRect() const -> QRectF
{
    const double widthMarginValue = widthMargin();
    const double heightMarginValue = heightMargin();

    const double w = width() - 2 * widthMarginValue;
    const double h = height() - 2 * heightMarginValue;
    return QRectF(widthMarginValue, heightMarginValue, w, h);
}

auto SwitchButton::thumbRect() const -> QRectF
{
    const double size = thumbSize();
    return QRectF(d_ptr->offset, heightMargin(), size, size);
}
