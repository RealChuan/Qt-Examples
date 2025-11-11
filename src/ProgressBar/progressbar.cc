#include "progressbar.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>

class ProgressBar::ProgressBarPrivate
{
public:
    explicit ProgressBarPrivate(ProgressBar *q)
        : q_ptr(q)
    {}

    ProgressBar *q_ptr;

    // 可配置属性
    double maxValue = 100.0;
    double minValue = 0.0;
    double value = minValue;
    double radius = 5.0;
    bool autoRadius = true;
    bool showPercent = true;

    // 颜色配置
    QColor chunkColor = QColor(77, 161, 255);
    QColor textColor = QColor(64, 65, 66);
    QColor baseColor = QColor(179, 179, 179);
    QColor backgroundColor = Qt::transparent;

    // 动画配置
    int animationDuration = 500;

    // 动画
    QPropertyAnimation *animation = nullptr;
    double targetValue = 0.0;

    // 字体配置
    static constexpr double FONT_SIZE_RATIO = 0.8;
};

ProgressBar::ProgressBar(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ProgressBarPrivate(this))
{
    d_ptr->animation = new QPropertyAnimation(this, "value", this);
    d_ptr->animation->setDuration(d_ptr->animationDuration);
    d_ptr->animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(d_ptr->animation,
            &QPropertyAnimation::finished,
            this,
            &ProgressBar::onAnimationFinished);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

ProgressBar::~ProgressBar() = default;

auto ProgressBar::minimumSizeHint() const -> QSize
{
    return {100, 10};
}

// Value
auto ProgressBar::value() const -> double
{
    return d_ptr->value;
}

void ProgressBar::setValue(double value)
{
    value = qBound(d_ptr->minValue, value, d_ptr->maxValue);

    if (qFuzzyCompare(value, d_ptr->value))
        return;

    const double oldValue = d_ptr->value;
    d_ptr->value = value;
    update();

    if (!isAnimating() || qFuzzyCompare(value, d_ptr->targetValue)) {
        emit valueChanged(value);

        if (value > oldValue) {
            emit valueIncreased(value);
        } else if (value < oldValue) {
            emit valueDecreased(value);
        }
    }
}

void ProgressBar::setValueAnimated(double value)
{
    value = qBound(d_ptr->minValue, value, d_ptr->maxValue);

    if (qFuzzyCompare(value, d_ptr->value))
        return;

    d_ptr->targetValue = value;
    emit animationStarted(d_ptr->value, value);
    startAnimation(value);
}

// Min value
void ProgressBar::setMinValue(double min)
{
    if (qFuzzyCompare(d_ptr->minValue, min))
        return;

    d_ptr->minValue = min;
    if (d_ptr->value < min) {
        setValue(min);
    }
    update();
}

auto ProgressBar::minValue() const -> double
{
    return d_ptr->minValue;
}

// Max value
void ProgressBar::setMaxValue(double max)
{
    if (qFuzzyCompare(d_ptr->maxValue, max))
        return;

    d_ptr->maxValue = max;
    if (d_ptr->value > max) {
        setValue(max);
    }
    update();
}

auto ProgressBar::maxValue() const -> double
{
    return d_ptr->maxValue;
}

// Radius
void ProgressBar::setRadius(double radius)
{
    if (qFuzzyCompare(d_ptr->radius, radius))
        return;

    d_ptr->radius = radius;
    update();
}

auto ProgressBar::radius() const -> double
{
    return d_ptr->radius;
}

// Auto radius
void ProgressBar::setAutoRadius(bool autoRadius)
{
    if (d_ptr->autoRadius == autoRadius)
        return;

    d_ptr->autoRadius = autoRadius;
    update();
}

auto ProgressBar::autoRadius() const -> bool
{
    return d_ptr->autoRadius;
}

// Show percent
void ProgressBar::setShowPercent(bool percent)
{
    if (d_ptr->showPercent == percent)
        return;

    d_ptr->showPercent = percent;
    update();
}

auto ProgressBar::showPercent() const -> bool
{
    return d_ptr->showPercent;
}

// Chunk color
void ProgressBar::setChunkColor(const QColor &color)
{
    if (d_ptr->chunkColor == color)
        return;

    d_ptr->chunkColor = color;
    update();
}

auto ProgressBar::chunkColor() const -> QColor
{
    return d_ptr->chunkColor;
}

// Text color
void ProgressBar::setTextColor(const QColor &color)
{
    if (d_ptr->textColor == color)
        return;

    d_ptr->textColor = color;
    update();
}

auto ProgressBar::textColor() const -> QColor
{
    return d_ptr->textColor;
}

// Base color
void ProgressBar::setBaseColor(const QColor &color)
{
    if (d_ptr->baseColor == color)
        return;

    d_ptr->baseColor = color;
    update();
}

auto ProgressBar::baseColor() const -> QColor
{
    return d_ptr->baseColor;
}

// Background color
void ProgressBar::setBackgroundColor(const QColor &color)
{
    if (d_ptr->backgroundColor == color)
        return;

    d_ptr->backgroundColor = color;
    update();
}

auto ProgressBar::backgroundColor() const -> QColor
{
    return d_ptr->backgroundColor;
}

// Animation duration
void ProgressBar::setAnimationDuration(int duration)
{
    if (duration < 0 || d_ptr->animationDuration == duration)
        return;

    d_ptr->animationDuration = duration;
    d_ptr->animation->setDuration(duration);
}

auto ProgressBar::animationDuration() const -> int
{
    return d_ptr->animationDuration;
}

// Animation state
bool ProgressBar::isAnimating() const
{
    return d_ptr->animation && d_ptr->animation->state() == QPropertyAnimation::Running;
}

// Public slots implementation
void ProgressBar::increaseValue(double increment)
{
    if (increment <= 0.0)
        return;

    const double newValue = qMin(d_ptr->maxValue, d_ptr->value + increment);
    if (qFuzzyCompare(newValue, d_ptr->value))
        return;

    setValueAnimated(newValue);
}

void ProgressBar::decreaseValue(double decrement)
{
    if (decrement <= 0.0)
        return;

    const double newValue = qMax(d_ptr->minValue, d_ptr->value - decrement);
    if (qFuzzyCompare(newValue, d_ptr->value))
        return;

    setValueAnimated(newValue);
}

void ProgressBar::reset()
{
    if (qFuzzyCompare(d_ptr->value, d_ptr->minValue))
        return;

    setValueAnimated(d_ptr->minValue);
    emit valueReset();
}

// Painting
void ProgressBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    if (d_ptr->backgroundColor != Qt::transparent) {
        painter.fillRect(rect(), d_ptr->backgroundColor);
    }

    drawProgressBar(&painter);
    drawText(&painter);
}

void ProgressBar::startAnimation(double targetValue)
{
    if (qFuzzyCompare(targetValue, d_ptr->value) || !d_ptr->animation)
        return;

    if (d_ptr->animation->state() == QPropertyAnimation::Running) {
        d_ptr->animation->stop();
    }

    d_ptr->animation->setStartValue(d_ptr->value);
    d_ptr->animation->setEndValue(targetValue);
    d_ptr->animation->start();
}

void ProgressBar::onAnimationFinished()
{
    emit animationFinished(d_ptr->value);

    if (qFuzzyCompare(d_ptr->value, d_ptr->targetValue)) {
        emit valueChanged(d_ptr->value);
    }
}

void ProgressBar::setupFont(QPainter *painter)
{
    const double fontSize = height() * d_ptr->FONT_SIZE_RATIO;
    auto font = painter->font();
    font.setPixelSize(static_cast<int>(fontSize));
    painter->setFont(font);
}

void ProgressBar::drawProgressBar(QPainter *painter)
{
    const QRectF baseRect = rect();
    double currentRadius = d_ptr->radius;

    // 自动计算圆角半径
    if (d_ptr->autoRadius) {
        currentRadius = qMin(baseRect.height(), baseRect.width()) / 2.0;
    }

    painter->setPen(Qt::NoPen);

    // 绘制背景
    painter->setBrush(d_ptr->baseColor);
    painter->drawRoundedRect(baseRect, currentRadius, currentRadius);

    // 绘制进度条
    if (d_ptr->value > d_ptr->minValue) {
        const double progressWidth = baseRect.width()
                                     * ((d_ptr->value - d_ptr->minValue)
                                        / (d_ptr->maxValue - d_ptr->minValue));

        drawProgressChunk(painter, baseRect, progressWidth, currentRadius);
    }
}

void ProgressBar::drawProgressChunk(QPainter *painter,
                                    const QRectF &baseRect,
                                    double progressWidth,
                                    double baseRadius)
{
    if (progressWidth <= 0)
        return;

    const double height = baseRect.height();

    painter->setBrush(d_ptr->chunkColor);

    painter->save();

    // 设置裁剪区域，只显示进度部分
    QPainterPath clipPath;
    clipPath.addRect(0, 0, progressWidth, height);
    painter->setClipPath(clipPath);

    // 创建一个完整的圆角矩形（左右都有圆角）
    // 宽度设为足够大，确保圆角完整显示
    const double fullWidth = qMax(progressWidth, 2 * baseRadius);
    QPainterPath fullRoundedPath;
    fullRoundedPath.addRoundedRect(0, 0, fullWidth, height, baseRadius, baseRadius);

    // 绘制完整的圆角矩形
    painter->drawPath(fullRoundedPath);

    painter->restore();
}

void ProgressBar::drawText(QPainter *painter)
{
    painter->save();

    setupFont(painter);
    painter->setPen(d_ptr->textColor);

    QString valueText;
    if (d_ptr->showPercent) {
        const double percent = (d_ptr->value - d_ptr->minValue)
                               / (d_ptr->maxValue - d_ptr->minValue) * 100.0;
        valueText = QString("%1%").arg(percent, 0, 'f', 2);
    } else {
        valueText = QString("%1").arg(d_ptr->value, 0, 'f', 2);
    }

    painter->drawText(rect(), Qt::AlignCenter, valueText);
    painter->restore();
}
