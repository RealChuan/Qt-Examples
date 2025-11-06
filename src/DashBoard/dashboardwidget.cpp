#include "dashboardwidget.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QtMath>

class DashBoardWidget::DashBoardWidgetPrivate
{
public:
    explicit DashBoardWidgetPrivate(DashBoardWidget *q)
        : q_ptr(q)
    {}

    DashBoardWidget *q_ptr;

    // 可配置属性
    double maxValue = 100.0;
    double minValue = 0.0;
    double startAngle = -50.0;
    double endAngle = 230.0;
    double value = minValue;
    int scaleMajor = 10;
    int scaleMinor = 5;
    QString unit = "unit";
    QString title = "test";

    // 颜色配置
    QColor arcColor = QColor(56, 61, 74);
    QColor scaleColor = QColor(4, 168, 173);
    QColor pointerColor = QColor(4, 181, 200);
    QColor textColor = QColor(144, 133, 116);
    QColor backgroundColor = Qt::transparent;
    QColor valueColor = QColor(144, 133, 116);
    QColor titleColor = QColor(144, 133, 116);

    // 动画配置
    int animationDuration = 500;

    // 动画
    QPropertyAnimation *animation = nullptr;
    double targetValue = 0.0;

    // 常量定义
    static constexpr double ARC_WIDTH_RATIO = 1.0 / 15.0;
    static constexpr double POINTER_WIDTH_RATIO = 1.0 / 60.0;
    static constexpr double POINTER_LENGTH_RATIO = 1.0 / 3.0;
    static constexpr double SCALE_MAJOR_LENGTH_RATIO = 1.0 / 20.0;
    static constexpr double SCALE_MINOR_LENGTH_RATIO = 1.0 / 30.0;
    static constexpr double VALUE_FONT_RATIO = 1.0 / 20.0;
    static constexpr double TITLE_FONT_RATIO = 1.0 / 20.0;
    static constexpr double SCALE_FONT_RATIO = 1.0 / 25.0;
};

DashBoardWidget::DashBoardWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new DashBoardWidgetPrivate(this))
{
    d_ptr->animation = new QPropertyAnimation(this, "value", this);
    d_ptr->animation->setDuration(d_ptr->animationDuration);
    d_ptr->animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(d_ptr->animation,
            &QPropertyAnimation::finished,
            this,
            &DashBoardWidget::onAnimationFinished);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

DashBoardWidget::~DashBoardWidget() = default;

auto DashBoardWidget::minimumSizeHint() const -> QSize
{
    return {150, 150};
}

// Value
auto DashBoardWidget::value() const -> double
{
    return d_ptr->value;
}

void DashBoardWidget::setValue(double value)
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

void DashBoardWidget::setValueAnimated(double value)
{
    value = qBound(d_ptr->minValue, value, d_ptr->maxValue);

    if (qFuzzyCompare(value, d_ptr->value))
        return;

    d_ptr->targetValue = value;
    emit animationStarted(d_ptr->value, value);
    startAnimation(value);
}

// Min value
void DashBoardWidget::setMinValue(double min)
{
    if (qFuzzyCompare(d_ptr->minValue, min))
        return;

    d_ptr->minValue = min;
    if (d_ptr->value < min) {
        setValue(min);
    }
    update();
}

auto DashBoardWidget::minValue() const -> double
{
    return d_ptr->minValue;
}

// Max value
void DashBoardWidget::setMaxValue(double max)
{
    if (qFuzzyCompare(d_ptr->maxValue, max))
        return;

    d_ptr->maxValue = max;
    if (d_ptr->value > max) {
        setValue(max);
    }
    update();
}

auto DashBoardWidget::maxValue() const -> double
{
    return d_ptr->maxValue;
}

// Start angle
void DashBoardWidget::setStartAngle(double startAngle)
{
    if (qFuzzyCompare(d_ptr->startAngle, startAngle))
        return;

    d_ptr->startAngle = startAngle;
    update();
}

auto DashBoardWidget::startAngle() const -> double
{
    return d_ptr->startAngle;
}

// End angle
void DashBoardWidget::setEndAngle(double endAngle)
{
    if (qFuzzyCompare(d_ptr->endAngle, endAngle))
        return;

    d_ptr->endAngle = endAngle;
    update();
}

auto DashBoardWidget::endAngle() const -> double
{
    return d_ptr->endAngle;
}

// Scale major
void DashBoardWidget::setScaleMajor(int scale)
{
    if (scale <= 0 || d_ptr->scaleMajor == scale)
        return;

    d_ptr->scaleMajor = scale;
    update();
}

auto DashBoardWidget::scaleMajor() const -> int
{
    return d_ptr->scaleMajor;
}

// Scale minor
void DashBoardWidget::setScaleMinor(int scale)
{
    if (scale <= 0 || d_ptr->scaleMinor == scale)
        return;

    d_ptr->scaleMinor = scale;
    update();
}

auto DashBoardWidget::scaleMinor() const -> int
{
    return d_ptr->scaleMinor;
}

// Unit
void DashBoardWidget::setUnit(const QString &unit)
{
    if (d_ptr->unit == unit)
        return;

    d_ptr->unit = unit;
    update();
}

auto DashBoardWidget::unit() const -> QString
{
    return d_ptr->unit;
}

// Title
void DashBoardWidget::setTitle(const QString &title)
{
    if (d_ptr->title == title)
        return;

    d_ptr->title = title;
    update();
}

auto DashBoardWidget::title() const -> QString
{
    return d_ptr->title;
}

// Arc color
void DashBoardWidget::setArcColor(const QColor &color)
{
    if (d_ptr->arcColor == color)
        return;

    d_ptr->arcColor = color;
    update();
}

auto DashBoardWidget::arcColor() const -> QColor
{
    return d_ptr->arcColor;
}

// Scale color
void DashBoardWidget::setScaleColor(const QColor &color)
{
    if (d_ptr->scaleColor == color)
        return;

    d_ptr->scaleColor = color;
    update();
}

auto DashBoardWidget::scaleColor() const -> QColor
{
    return d_ptr->scaleColor;
}

// Pointer color
void DashBoardWidget::setPointerColor(const QColor &color)
{
    if (d_ptr->pointerColor == color)
        return;

    d_ptr->pointerColor = color;
    update();
}

auto DashBoardWidget::pointerColor() const -> QColor
{
    return d_ptr->pointerColor;
}

// Text color
void DashBoardWidget::setTextColor(const QColor &color)
{
    if (d_ptr->textColor == color)
        return;

    d_ptr->textColor = color;
    update();
}

auto DashBoardWidget::textColor() const -> QColor
{
    return d_ptr->textColor;
}

// Background color
void DashBoardWidget::setBackgroundColor(const QColor &color)
{
    if (d_ptr->backgroundColor == color)
        return;

    d_ptr->backgroundColor = color;
    update();
}

auto DashBoardWidget::backgroundColor() const -> QColor
{
    return d_ptr->backgroundColor;
}

// Value color
void DashBoardWidget::setValueColor(const QColor &color)
{
    if (d_ptr->valueColor == color)
        return;

    d_ptr->valueColor = color;
    update();
}

auto DashBoardWidget::valueColor() const -> QColor
{
    return d_ptr->valueColor;
}

// Title color
void DashBoardWidget::setTitleColor(const QColor &color)
{
    if (d_ptr->titleColor == color)
        return;

    d_ptr->titleColor = color;
    update();
}

auto DashBoardWidget::titleColor() const -> QColor
{
    return d_ptr->titleColor;
}

// Animation duration
void DashBoardWidget::setAnimationDuration(int duration)
{
    if (duration < 0 || d_ptr->animationDuration == duration)
        return;

    d_ptr->animationDuration = duration;
    d_ptr->animation->setDuration(duration);
}

auto DashBoardWidget::animationDuration() const -> int
{
    return d_ptr->animationDuration;
}

// Animation state
bool DashBoardWidget::isAnimating() const
{
    return d_ptr->animation && d_ptr->animation->state() == QPropertyAnimation::Running;
}

// Public slots implementation
void DashBoardWidget::increaseValue(double increment)
{
    if (increment <= 0.0)
        return;

    const double newValue = qMin(d_ptr->maxValue, d_ptr->value + increment);
    if (qFuzzyCompare(newValue, d_ptr->value))
        return;

    setValueAnimated(newValue);
}

void DashBoardWidget::decreaseValue(double decrement)
{
    if (decrement <= 0.0)
        return;

    const double newValue = qMax(d_ptr->minValue, d_ptr->value - decrement);
    if (qFuzzyCompare(newValue, d_ptr->value))
        return;

    setValueAnimated(newValue);
}

void DashBoardWidget::reset()
{
    if (qFuzzyCompare(d_ptr->value, d_ptr->minValue))
        return;

    setValueAnimated(d_ptr->minValue);
    emit valueReset();
}

// Painting
void DashBoardWidget::paintEvent(QPaintEvent *event)
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
    drawScale(&painter, minSize);
    drawScaleNumbers(&painter, minSize);
    drawPointer(&painter, minSize);
    drawValue(&painter, minSize);
    drawTitle(&painter, minSize);
}

void DashBoardWidget::startAnimation(double targetValue)
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

void DashBoardWidget::onAnimationFinished()
{
    emit animationFinished(d_ptr->value);

    if (qFuzzyCompare(d_ptr->value, d_ptr->targetValue)) {
        emit valueChanged(d_ptr->value);
    }
}

void DashBoardWidget::setupFont(QPainter *painter, double minSize, double ratio)
{
    auto font = painter->font();
    font.setPixelSize(static_cast<int>(minSize * ratio));
    painter->setFont(font);
}

QRectF DashBoardWidget::getTextRect(double minSize, double verticalRatio, double heightRatio) const
{
    const double radius = minSize / 2.0 - minSize / 4.8;
    return QRectF(-radius, radius * verticalRatio, radius * 2, radius * heightRatio);
}

void DashBoardWidget::drawArc(QPainter *painter, double minSize)
{
    const double arcWidth = minSize * DashBoardWidgetPrivate::ARC_WIDTH_RATIO;
    const double radius = minSize / 3 - arcWidth;
    const QRectF rect = QRectF(-radius, -radius, radius * 2, radius * 2);

    QPen pen;
    pen.setWidthF(arcWidth);
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(d_ptr->arcColor);

    const double angle = d_ptr->endAngle - d_ptr->startAngle;

    painter->save();
    painter->setPen(pen);
    painter->drawArc(rect, d_ptr->startAngle * 16, angle * 16);
    painter->restore();
}

void DashBoardWidget::drawScale(QPainter *painter, double minSize)
{
    painter->save();
    painter->rotate(270 - d_ptr->endAngle);

    const int steps = (d_ptr->scaleMajor * d_ptr->scaleMinor);
    const double angleStep = (d_ptr->endAngle - d_ptr->startAngle) / steps;
    const double radius = minSize / 3;

    QPen pen(d_ptr->scaleColor);
    pen.setCapStyle(Qt::RoundCap);

    for (int i = 0; i <= steps; i++) {
        if (i % d_ptr->scaleMinor == 0) {
            pen.setWidthF(1.5);
            painter->setPen(pen);
            painter->drawLine(0, radius - 8, 0, radius + 5);
        } else {
            pen.setWidthF(0.5);
            painter->setPen(pen);
            painter->drawLine(0, radius - 8, 0, radius - 3);
        }

        painter->rotate(angleStep);
    }

    painter->restore();
}

void DashBoardWidget::drawScaleNumbers(QPainter *painter, double minSize)
{
    painter->save();
    painter->setPen(d_ptr->scaleColor);

    setupFont(painter, minSize, DashBoardWidgetPrivate::SCALE_FONT_RATIO);

    const double radius = minSize / 2.4;
    const double startRad = d_ptr->endAngle * (M_PI / 180);
    const double deltaRad = (d_ptr->endAngle - d_ptr->startAngle) * (M_PI / 180)
                            / d_ptr->scaleMajor;

    const QFontMetrics fontMetrics(painter->font());
    for (int i = 0; i <= d_ptr->scaleMajor; i++) {
        const double sina = qSin(startRad - i * deltaRad);
        const double cosa = qCos(startRad - i * deltaRad);
        const double value = 1.0 * i * ((d_ptr->maxValue - d_ptr->minValue) / d_ptr->scaleMajor)
                             + d_ptr->minValue;

        const QString strValue = QString("%1").arg(value, 0, 'f', 2);
        const double textWidth = fontMetrics.horizontalAdvance(strValue);
        const double textHeight = fontMetrics.height();
        const int x = radius * cosa - textWidth / 2;
        const int y = -radius * sina + textHeight / 4;
        painter->drawText(x, y, strValue);
    }

    painter->restore();
}

void DashBoardWidget::drawPointer(QPainter *painter, double minSize)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->pointerColor);

    const double radius = minSize / 3.0;

    QPolygonF pts;
    pts << QPointF(-5, 0) << QPointF(0, -8) << QPointF(5, 0) << QPointF(0, radius);

    painter->rotate(270 - d_ptr->endAngle);
    const double degRotate = (d_ptr->endAngle - d_ptr->startAngle)
                             / (d_ptr->maxValue - d_ptr->minValue)
                             * (d_ptr->value - d_ptr->minValue);
    painter->rotate(degRotate);
    painter->drawConvexPolygon(pts);

    painter->restore();
}

void DashBoardWidget::drawValue(QPainter *painter, double minSize)
{
    painter->save();
    painter->setPen(d_ptr->valueColor);

    setupFont(painter, minSize, DashBoardWidgetPrivate::VALUE_FONT_RATIO);

    const QString strValue = QString("%1 %2").arg(d_ptr->value, 0, 'f', 2).arg(d_ptr->unit);
    const QRectF valueRect = getTextRect(minSize, 1.0 / 2.5, 1.0 / 3.5);
    painter->drawText(valueRect, Qt::AlignCenter, strValue);

    painter->restore();
}

void DashBoardWidget::drawTitle(QPainter *painter, double minSize)
{
    painter->save();
    painter->setPen(d_ptr->titleColor);

    setupFont(painter, minSize, DashBoardWidgetPrivate::TITLE_FONT_RATIO);

    const QRectF textRect = getTextRect(minSize, 1.0 / 1.5, 1.0 / 2.5);
    painter->drawText(textRect, Qt::AlignCenter, d_ptr->title);

    painter->restore();
}
