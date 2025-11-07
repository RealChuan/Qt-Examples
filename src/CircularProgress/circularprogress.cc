#include "circularprogress.hpp"

#include <QPainter>
#include <QPropertyAnimation>
#include <QtMath>

class CircularProgress::CircularProgressPrivate
{
public:
    explicit CircularProgressPrivate(CircularProgress *q)
        : q_ptr(q)
    {}

    CircularProgress *q_ptr;

    // 可配置属性
    double maxValue = 100.0;
    double minValue = 0.0;
    double startAngle = -50.0;
    double endAngle = 230.0;
    double value = minValue;
    bool showPercent = true;
    QString title = "Progress";

    // 颜色配置
    QColor arcColor = QColor(77, 161, 255);
    QColor textColor = QColor(77, 161, 255);
    QColor titleColor = QColor(80, 80, 80);
    QColor baseColor = QColor(179, 179, 179);
    QColor backgroundColor = Qt::transparent;

    // 动画配置
    int animationDuration = 500;

    // 动画
    QPropertyAnimation *animation = nullptr;
    double targetValue = 0.0;

    // 常量定义
    static constexpr double ARC_WIDTH_RATIO = 1.0 / 10.0;
    static constexpr double VALUE_FONT_RATIO = 1.0 / 10.0;
    static constexpr double TITLE_FONT_RATIO = 1.0 / 15.0;
    static constexpr double ARC_MARGIN_RATIO = 1.0 / 20.0;
};

CircularProgress::CircularProgress(const QString &title, QWidget *parent)
    : QWidget(parent)
    , d_ptr(new CircularProgressPrivate(this))
{
    d_ptr->title = title;
    d_ptr->animation = new QPropertyAnimation(this, "value", this);
    d_ptr->animation->setDuration(d_ptr->animationDuration);
    d_ptr->animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(d_ptr->animation,
            &QPropertyAnimation::finished,
            this,
            &CircularProgress::onAnimationFinished);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

CircularProgress::~CircularProgress() = default;

auto CircularProgress::minimumSizeHint() const -> QSize
{
    return {80, 80};
}

// Value
auto CircularProgress::value() const -> double
{
    return d_ptr->value;
}

void CircularProgress::setValue(double value)
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

void CircularProgress::setValueAnimated(double value)
{
    value = qBound(d_ptr->minValue, value, d_ptr->maxValue);

    if (qFuzzyCompare(value, d_ptr->value))
        return;

    d_ptr->targetValue = value;
    emit animationStarted(d_ptr->value, value);
    startAnimation(value);
}

// Min value
void CircularProgress::setMinValue(double min)
{
    if (qFuzzyCompare(d_ptr->minValue, min))
        return;

    d_ptr->minValue = min;
    if (d_ptr->value < min) {
        setValue(min);
    }
    update();
}

auto CircularProgress::minValue() const -> double
{
    return d_ptr->minValue;
}

// Max value
void CircularProgress::setMaxValue(double max)
{
    if (qFuzzyCompare(d_ptr->maxValue, max))
        return;

    d_ptr->maxValue = max;
    if (d_ptr->value > max) {
        setValue(max);
    }
    update();
}

auto CircularProgress::maxValue() const -> double
{
    return d_ptr->maxValue;
}

// Start angle
void CircularProgress::setStartAngle(double startAngle)
{
    if (qFuzzyCompare(d_ptr->startAngle, startAngle))
        return;

    d_ptr->startAngle = startAngle;
    update();
}

auto CircularProgress::startAngle() const -> double
{
    return d_ptr->startAngle;
}

// End angle
void CircularProgress::setEndAngle(double endAngle)
{
    if (qFuzzyCompare(d_ptr->endAngle, endAngle))
        return;

    d_ptr->endAngle = endAngle;
    update();
}

auto CircularProgress::endAngle() const -> double
{
    return d_ptr->endAngle;
}

// Show percent
void CircularProgress::setShowPercent(bool percent)
{
    if (d_ptr->showPercent == percent)
        return;

    d_ptr->showPercent = percent;
    update();
}

auto CircularProgress::showPercent() const -> bool
{
    return d_ptr->showPercent;
}

// Title
void CircularProgress::setTitle(const QString &title)
{
    if (d_ptr->title == title)
        return;

    d_ptr->title = title;
    update();
}

auto CircularProgress::title() const -> QString
{
    return d_ptr->title;
}

// Arc color
void CircularProgress::setArcColor(const QColor &color)
{
    if (d_ptr->arcColor == color)
        return;

    d_ptr->arcColor = color;
    update();
}

auto CircularProgress::arcColor() const -> QColor
{
    return d_ptr->arcColor;
}

// Text color
void CircularProgress::setTextColor(const QColor &color)
{
    if (d_ptr->textColor == color)
        return;

    d_ptr->textColor = color;
    update();
}

auto CircularProgress::textColor() const -> QColor
{
    return d_ptr->textColor;
}

// Title color
void CircularProgress::setTitleColor(const QColor &color)
{
    if (d_ptr->titleColor == color)
        return;

    d_ptr->titleColor = color;
    update();
}

auto CircularProgress::titleColor() const -> QColor
{
    return d_ptr->titleColor;
}

// Base color
void CircularProgress::setBaseColor(const QColor &color)
{
    if (d_ptr->baseColor == color)
        return;

    d_ptr->baseColor = color;
    update();
}

auto CircularProgress::baseColor() const -> QColor
{
    return d_ptr->baseColor;
}

// Background color
void CircularProgress::setBackgroundColor(const QColor &color)
{
    if (d_ptr->backgroundColor == color)
        return;

    d_ptr->backgroundColor = color;
    update();
}

auto CircularProgress::backgroundColor() const -> QColor
{
    return d_ptr->backgroundColor;
}

// Animation duration
void CircularProgress::setAnimationDuration(int duration)
{
    if (duration < 0 || d_ptr->animationDuration == duration)
        return;

    d_ptr->animationDuration = duration;
    d_ptr->animation->setDuration(duration);
}

auto CircularProgress::animationDuration() const -> int
{
    return d_ptr->animationDuration;
}

// Animation state
bool CircularProgress::isAnimating() const
{
    return d_ptr->animation && d_ptr->animation->state() == QPropertyAnimation::Running;
}

// Public slots implementation
void CircularProgress::increaseValue(double increment)
{
    if (increment <= 0.0)
        return;

    const double newValue = qMin(d_ptr->maxValue, d_ptr->value + increment);
    if (qFuzzyCompare(newValue, d_ptr->value))
        return;

    setValueAnimated(newValue);
}

void CircularProgress::decreaseValue(double decrement)
{
    if (decrement <= 0.0)
        return;

    const double newValue = qMax(d_ptr->minValue, d_ptr->value - decrement);
    if (qFuzzyCompare(newValue, d_ptr->value))
        return;

    setValueAnimated(newValue);
}

void CircularProgress::reset()
{
    if (qFuzzyCompare(d_ptr->value, d_ptr->minValue))
        return;

    setValueAnimated(d_ptr->minValue);
    emit valueReset();
}

// Painting
void CircularProgress::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    if (d_ptr->backgroundColor != Qt::transparent) {
        painter.fillRect(rect(), d_ptr->backgroundColor);
    }

    painter.translate(width() / 2.0, height() / 2.0);

    const double minSize = qMin(width(), height());

    drawArc(&painter, minSize);
    drawText(&painter, minSize);
}

void CircularProgress::startAnimation(double targetValue)
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

void CircularProgress::onAnimationFinished()
{
    emit animationFinished(d_ptr->value);

    if (qFuzzyCompare(d_ptr->value, d_ptr->targetValue)) {
        emit valueChanged(d_ptr->value);
    }
}

void CircularProgress::setupFont(QPainter *painter, double minSize, double ratio)
{
    auto font = painter->font();
    font.setPixelSize(static_cast<int>(minSize * ratio));
    painter->setFont(font);
}

QRectF CircularProgress::getValueRect(double minSize) const
{
    const double radius = minSize / 2.0 - minSize * d_ptr->ARC_WIDTH_RATIO
                          - minSize * d_ptr->ARC_MARGIN_RATIO;
    return QRectF(-radius, 0, radius * 2, radius / 3);
}

QRectF CircularProgress::getTitleRect(double minSize) const
{
    const double radius = minSize / 2.0 - minSize * d_ptr->ARC_WIDTH_RATIO
                          - minSize * d_ptr->ARC_MARGIN_RATIO;
    return QRectF(-radius, -radius / 2.5, radius * 2, radius / 3);
}

void CircularProgress::drawArc(QPainter *painter, double minSize)
{
    const double arcWidth = minSize * d_ptr->ARC_WIDTH_RATIO;
    const double margin = minSize * d_ptr->ARC_MARGIN_RATIO;
    const double radius = minSize / 2.0 - arcWidth - margin;
    const QRectF rect = QRectF(-radius, -radius, radius * 2, radius * 2);

    QPen pen;
    pen.setWidthF(arcWidth);
    pen.setCapStyle(Qt::RoundCap);

    // 绘制背景圆弧
    const double totalAngle = d_ptr->endAngle - d_ptr->startAngle;
    pen.setColor(d_ptr->baseColor);
    painter->setPen(pen);
    painter->drawArc(rect, d_ptr->startAngle * 16, totalAngle * 16);

    // 绘制进度圆弧
    if (d_ptr->value > d_ptr->minValue) {
        const double progressAngle = totalAngle
                                     * ((d_ptr->value - d_ptr->minValue)
                                        / (d_ptr->maxValue - d_ptr->minValue));
        const double startAngle = d_ptr->endAngle - progressAngle;

        pen.setColor(d_ptr->arcColor);
        painter->setPen(pen);
        painter->drawArc(rect, startAngle * 16, progressAngle * 16);
    }
}

void CircularProgress::drawText(QPainter *painter, double minSize)
{
    painter->save();

    // 绘制数值
    setupFont(painter, minSize, d_ptr->VALUE_FONT_RATIO);
    painter->setPen(d_ptr->textColor);

    QString valueText;
    if (d_ptr->showPercent) {
        const double percent = (d_ptr->value - d_ptr->minValue)
                               / (d_ptr->maxValue - d_ptr->minValue) * 100.0;
        valueText = QString("%1%").arg(percent, 0, 'f', 2);
    } else {
        valueText = QString("%1").arg(d_ptr->value, 0, 'f', 2);
    }

    const QRectF valueRect = getValueRect(minSize);
    painter->drawText(valueRect, Qt::AlignCenter, valueText);

    // 绘制标题
    setupFont(painter, minSize, d_ptr->TITLE_FONT_RATIO);
    painter->setPen(d_ptr->titleColor);

    const QRectF titleRect = getTitleRect(minSize);
    painter->drawText(titleRect, Qt::AlignCenter, d_ptr->title);

    painter->restore();
}
