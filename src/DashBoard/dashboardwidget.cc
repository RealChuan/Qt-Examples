#include "dashboardwidget.hpp"

#include <QPainter>
#include <QPropertyAnimation>
#include <QtMath>

using namespace Qt::StringLiterals;

class DashBoardWidget::DashBoardWidgetPrivate
{
public:
    explicit DashBoardWidgetPrivate(DashBoardWidget *q) : q_ptr(q) {}

    DashBoardWidget *q_ptr;

    // 可配置属性
    double maxValue = 100.0;
    double minValue = 0.0;
    double startAngle = -50.0;
    double endAngle = 230.0;
    double value = minValue;
    int scaleMajor = 10;
    int scaleMinor = 5;
    QString unit = u"km/h"_s;
    QString title = u"SPEED"_s;

    // Classic 主题默认颜色
    QColor arcColor = QColor(56, 61, 74);      // #383d4a
    QColor scaleColor = QColor(4, 168, 173);   // #04a8ad
    QColor pointerColor = QColor(4, 181, 200); // #04b5c8
    QColor textColor = QColor(144, 133, 116);  // #908574
    QColor backgroundColor = Qt::transparent;
    QColor valueColor = QColor(144, 133, 116); // #908574
    QColor titleColor = QColor(144, 133, 116); // #908574

    // 动画配置
    int animationDuration = 500;

    // 动画
    QPropertyAnimation *animation = nullptr;
    double targetValue = 0.0;

    // 比例常量
    static constexpr double ARC_WIDTH_RATIO = 1.0 / 20.0;
    static constexpr double ARC_RADIUS_RATIO = 0.38;
    static constexpr double SCALE_RADIUS_RATIO = 0.40;
    static constexpr double SCALE_MAJOR_IN_LENGTH_RATIO = 1.0 / 15.0;
    static constexpr double SCALE_MAJOR_OUT_LENGTH_RATIO = 1.0 / 30.0;
    static constexpr double SCALE_MINOR_LENGTH_RATIO = 1.0 / 45.0;
    static constexpr double SCALE_NUMBER_RADIUS_RATIO = 0.46;
    static constexpr double POINTER_LENGTH_RATIO = 0.35;
    static constexpr double POINTER_WIDTH_RATIO = 1.0 / 100.0;
    static constexpr double POINTER_TAIL_RATIO = 1.0 / 30.0;
    static constexpr double VALUE_FONT_RATIO = 1.0 / 15.0;
    static constexpr double TITLE_FONT_RATIO = 1.0 / 25.0;
    static constexpr double SCALE_FONT_RATIO = 1.0 / 25.0;
    static constexpr double HUB_RADIUS_RATIO = 1.0 / 40.0;
    static constexpr double HUB_INNER_RATIO = 0.4;
    static constexpr double VALUE_Y_RATIO = 0.15;
    static constexpr double TITLE_Y_RATIO = 0.25;
};

DashBoardWidget::DashBoardWidget(QWidget *parent)
    : QWidget(parent), d_ptr(std::make_unique<DashBoardWidgetPrivate>(this))
{
    initAnimations();

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

DashBoardWidget::~DashBoardWidget() = default;

auto DashBoardWidget::minimumSizeHint() const -> QSize
{ return {150, 150}; }

void DashBoardWidget::initAnimations()
{
    d_ptr->animation = new QPropertyAnimation(this, "value", this);
    d_ptr->animation->setDuration(d_ptr->animationDuration);
    d_ptr->animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(d_ptr->animation,
            &QPropertyAnimation::finished,
            this,
            &DashBoardWidget::onAnimationFinished);
}

// Value
auto DashBoardWidget::value() const -> double
{ return d_ptr->value; }

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
    emit minValueChanged(min);
}

auto DashBoardWidget::minValue() const -> double
{ return d_ptr->minValue; }

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
    emit maxValueChanged(max);
}

auto DashBoardWidget::maxValue() const -> double
{ return d_ptr->maxValue; }

// Start angle
void DashBoardWidget::setStartAngle(double startAngle)
{
    if (qFuzzyCompare(d_ptr->startAngle, startAngle))
        return;

    d_ptr->startAngle = startAngle;
    update();
    emit startAngleChanged(startAngle);
}

auto DashBoardWidget::startAngle() const -> double
{ return d_ptr->startAngle; }

// End angle
void DashBoardWidget::setEndAngle(double endAngle)
{
    if (qFuzzyCompare(d_ptr->endAngle, endAngle))
        return;

    d_ptr->endAngle = endAngle;
    update();
    emit endAngleChanged(endAngle);
}

auto DashBoardWidget::endAngle() const -> double
{ return d_ptr->endAngle; }

// Scale major
void DashBoardWidget::setScaleMajor(int scale)
{
    if (scale <= 0 || d_ptr->scaleMajor == scale)
        return;

    d_ptr->scaleMajor = scale;
    update();
    emit scaleMajorChanged(scale);
}

auto DashBoardWidget::scaleMajor() const -> int
{ return d_ptr->scaleMajor; }

// Scale minor
void DashBoardWidget::setScaleMinor(int scale)
{
    if (scale <= 0 || d_ptr->scaleMinor == scale)
        return;

    d_ptr->scaleMinor = scale;
    update();
    emit scaleMinorChanged(scale);
}

auto DashBoardWidget::scaleMinor() const -> int
{ return d_ptr->scaleMinor; }

// Unit
void DashBoardWidget::setUnit(const QString &unit)
{
    if (d_ptr->unit == unit)
        return;

    d_ptr->unit = unit;
    update();
    emit unitChanged(unit);
}

auto DashBoardWidget::unit() const -> QString
{ return d_ptr->unit; }

// Title
void DashBoardWidget::setTitle(const QString &title)
{
    if (d_ptr->title == title)
        return;

    d_ptr->title = title;
    update();
    emit titleChanged(title);
}

auto DashBoardWidget::title() const -> QString
{ return d_ptr->title; }

// Arc color
void DashBoardWidget::setArcColor(const QColor &color)
{
    if (d_ptr->arcColor == color)
        return;

    d_ptr->arcColor = color;
    update();
    emit arcColorChanged(color);
}

auto DashBoardWidget::arcColor() const -> QColor
{ return d_ptr->arcColor; }

// Scale color
void DashBoardWidget::setScaleColor(const QColor &color)
{
    if (d_ptr->scaleColor == color)
        return;

    d_ptr->scaleColor = color;
    update();
    emit scaleColorChanged(color);
}

auto DashBoardWidget::scaleColor() const -> QColor
{ return d_ptr->scaleColor; }

// Pointer color
void DashBoardWidget::setPointerColor(const QColor &color)
{
    if (d_ptr->pointerColor == color)
        return;

    d_ptr->pointerColor = color;
    update();
    emit pointerColorChanged(color);
}

auto DashBoardWidget::pointerColor() const -> QColor
{ return d_ptr->pointerColor; }

// Text color
void DashBoardWidget::setTextColor(const QColor &color)
{
    if (d_ptr->textColor == color)
        return;

    d_ptr->textColor = color;
    update();
    emit textColorChanged(color);
}

auto DashBoardWidget::textColor() const -> QColor
{ return d_ptr->textColor; }

// Background color
void DashBoardWidget::setBackgroundColor(const QColor &color)
{
    if (d_ptr->backgroundColor == color)
        return;

    d_ptr->backgroundColor = color;
    update();
    emit backgroundColorChanged(color);
}

auto DashBoardWidget::backgroundColor() const -> QColor
{ return d_ptr->backgroundColor; }

// Value color
void DashBoardWidget::setValueColor(const QColor &color)
{
    if (d_ptr->valueColor == color)
        return;

    d_ptr->valueColor = color;
    update();
    emit valueColorChanged(color);
}

auto DashBoardWidget::valueColor() const -> QColor
{ return d_ptr->valueColor; }

// Title color
void DashBoardWidget::setTitleColor(const QColor &color)
{
    if (d_ptr->titleColor == color)
        return;

    d_ptr->titleColor = color;
    update();
    emit titleColorChanged(color);
}

auto DashBoardWidget::titleColor() const -> QColor
{ return d_ptr->titleColor; }

// Animation duration
void DashBoardWidget::setAnimationDuration(int duration)
{
    if (duration < 0 || d_ptr->animationDuration == duration)
        return;

    d_ptr->animationDuration = duration;
    d_ptr->animation->setDuration(duration);
    emit animationDurationChanged(duration);
}

auto DashBoardWidget::animationDuration() const -> int
{ return d_ptr->animationDuration; }

// Animation state
bool DashBoardWidget::isAnimating() const
{ return d_ptr->animation && d_ptr->animation->state() == QPropertyAnimation::Running; }

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

    const double minSize = qMin(width(), height());

    painter.translate(width() / 2.0, height() / 2.0);

    drawArc(painter, minSize);
    drawProgressArc(painter, minSize);
    drawScale(painter, minSize);
    drawScaleNumbers(painter, minSize);
    drawPointer(painter, minSize);
    drawCenterHub(painter, minSize);
    drawValue(painter, minSize);
    drawTitle(painter, minSize);
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

void DashBoardWidget::setupFont(QPainter &painter, double minSize, double ratio)
{
    auto font = painter.font();
    font.setFamily(u"Consolas"_s);
    font.setStyleHint(QFont::Monospace, QFont::PreferMatch);
    font.setBold(true);
    font.setPixelSize(static_cast<int>(minSize * ratio));
    painter.setFont(font);
}

void DashBoardWidget::drawArc(QPainter &painter, double minSize)
{
    const double arcWidth = minSize * DashBoardWidgetPrivate::ARC_WIDTH_RATIO;
    const double radius = minSize * DashBoardWidgetPrivate::ARC_RADIUS_RATIO - arcWidth / 2;
    const QRectF rect = QRectF(-radius, -radius, radius * 2, radius * 2);

    QPen pen;
    pen.setWidthF(arcWidth);
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(d_ptr->arcColor);

    const double angle = d_ptr->endAngle - d_ptr->startAngle;

    painter.save();
    painter.setPen(pen);
    painter.drawArc(rect, d_ptr->startAngle * 16, angle * 16);
    painter.restore();
}

void DashBoardWidget::drawProgressArc(QPainter &painter, double minSize)
{
    const double range = d_ptr->maxValue - d_ptr->minValue;
    if (range <= 0)
        return;

    const double progress = (d_ptr->value - d_ptr->minValue) / range;
    const double progressSpan = (d_ptr->endAngle - d_ptr->startAngle) * progress;
    if (progressSpan <= 0)
        return;

    const double arcWidth = minSize * DashBoardWidgetPrivate::ARC_WIDTH_RATIO;
    const double radius = minSize * DashBoardWidgetPrivate::ARC_RADIUS_RATIO - arcWidth / 2;
    const QRectF rect = QRectF(-radius, -radius, radius * 2, radius * 2);

    const int startAngle16 = static_cast<int>(d_ptr->endAngle * 16);
    const int spanAngle16 = -static_cast<int>(progressSpan * 16);

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(d_ptr->valueColor);
    pen.setWidthF(arcWidth);
    painter.setPen(pen);
    painter.drawArc(rect, startAngle16, spanAngle16);
}

void DashBoardWidget::drawScale(QPainter &painter, double minSize)
{
    painter.save();
    painter.rotate(270 - d_ptr->endAngle);

    const int steps = (d_ptr->scaleMajor * d_ptr->scaleMinor);
    const double angleStep = (d_ptr->endAngle - d_ptr->startAngle) / steps;
    const double radius = minSize * DashBoardWidgetPrivate::SCALE_RADIUS_RATIO;
    const double majorInLen = minSize * DashBoardWidgetPrivate::SCALE_MAJOR_IN_LENGTH_RATIO;
    const double majorOutLen = minSize * DashBoardWidgetPrivate::SCALE_MAJOR_OUT_LENGTH_RATIO;
    const double minorLen = minSize * DashBoardWidgetPrivate::SCALE_MINOR_LENGTH_RATIO;

    QPen pen(d_ptr->scaleColor);
    pen.setCapStyle(Qt::FlatCap);

    QColor minorColor = d_ptr->scaleColor;
    minorColor.setAlpha(100);

    for (int i = 0; i <= steps; i++) {
        if (i % d_ptr->scaleMinor == 0) {
            // 主刻度：双向延伸（向内长 + 向外短）
            pen.setColor(d_ptr->scaleColor);
            pen.setWidthF(2.0);
            painter.setPen(pen);
            painter.drawLine(0, radius - majorInLen, 0, radius + majorOutLen);
        } else {
            // 次刻度：单向向内
            pen.setColor(minorColor);
            pen.setWidthF(1.0);
            painter.setPen(pen);
            painter.drawLine(0, radius - minorLen, 0, radius);
        }

        painter.rotate(angleStep);
    }

    painter.restore();
}

void DashBoardWidget::drawScaleNumbers(QPainter &painter, double minSize)
{
    painter.save();
    painter.setPen(d_ptr->textColor);

    setupFont(painter, minSize, DashBoardWidgetPrivate::SCALE_FONT_RATIO);
    auto font = painter.font();
    font.setBold(false);
    painter.setFont(font);

    const double radius = minSize * DashBoardWidgetPrivate::SCALE_NUMBER_RADIUS_RATIO;
    const double startRad = d_ptr->endAngle * (M_PI / 180);
    const double deltaRad
        = (d_ptr->endAngle - d_ptr->startAngle) * (M_PI / 180) / d_ptr->scaleMajor;

    const QFontMetrics fontMetrics(painter.font());
    for (int i = 0; i <= d_ptr->scaleMajor; i++) {
        const double sina = qSin(startRad - i * deltaRad);
        const double cosa = qCos(startRad - i * deltaRad);
        const double value
            = 1.0 * i * ((d_ptr->maxValue - d_ptr->minValue) / d_ptr->scaleMajor) + d_ptr->minValue;

        const QString strValue = u"%1"_s.arg(value, 0, 'f', 0);
        const double textWidth = fontMetrics.horizontalAdvance(strValue);
        const double textHeight = fontMetrics.height();
        const int x = radius * cosa - textWidth / 2;
        const int y = -radius * sina + textHeight / 4;
        painter.drawText(x, y, strValue);
    }

    painter.restore();
}

void DashBoardWidget::drawPointer(QPainter &painter, double minSize)
{
    painter.save();

    const double length = minSize * DashBoardWidgetPrivate::POINTER_LENGTH_RATIO;
    const double pw = minSize * DashBoardWidgetPrivate::POINTER_WIDTH_RATIO;
    const double tail = minSize * DashBoardWidgetPrivate::POINTER_TAIL_RATIO;

    painter.rotate(270 - d_ptr->endAngle);
    const double degRotate = (d_ptr->endAngle - d_ptr->startAngle)
                             / (d_ptr->maxValue - d_ptr->minValue)
                             * (d_ptr->value - d_ptr->minValue);
    painter.rotate(degRotate);

    // 细长剑形指针：长端指向弧方向，短尾部
    QPolygonF pts;
    pts << QPointF(-pw, 0) << QPointF(0, length) << QPointF(pw, 0) << QPointF(0, -tail);
    painter.setPen(Qt::NoPen);
    painter.setBrush(d_ptr->pointerColor);
    painter.drawConvexPolygon(pts);

    painter.restore();
}

void DashBoardWidget::drawCenterHub(QPainter &painter, double minSize)
{
    painter.save();
    painter.setPen(Qt::NoPen);

    const double hubRadius = minSize * DashBoardWidgetPrivate::HUB_RADIUS_RATIO;

    // 六边形外环（赛博朋克几何感）
    QPolygonF hexagon;
    for (int i = 0; i < 6; ++i) {
        const double angle = (60.0 * i - 90.0) * M_PI / 180.0;
        hexagon << QPointF(hubRadius * qCos(angle), hubRadius * qSin(angle));
    }
    painter.setBrush(d_ptr->pointerColor);
    painter.drawPolygon(hexagon);

    // 圆形内核
    painter.setBrush(d_ptr->valueColor);
    painter.drawEllipse(QPointF(0, 0),
                        hubRadius * DashBoardWidgetPrivate::HUB_INNER_RATIO,
                        hubRadius * DashBoardWidgetPrivate::HUB_INNER_RATIO);

    painter.restore();
}

void DashBoardWidget::drawValue(QPainter &painter, double minSize)
{
    painter.save();
    painter.setPen(d_ptr->valueColor);

    setupFont(painter, minSize, DashBoardWidgetPrivate::VALUE_FONT_RATIO);

    const QString strValue = u"%1 %2"_s.arg(d_ptr->value, 0, 'f', 2).arg(d_ptr->unit);
    const QFontMetrics fm(painter.font());
    const double textWidth = fm.horizontalAdvance(strValue);
    const double textHeight = fm.height();
    const double centerY = minSize * DashBoardWidgetPrivate::VALUE_Y_RATIO;
    const QRectF textRect(-textWidth / 2, centerY - textHeight / 2, textWidth, textHeight);
    painter.drawText(textRect, Qt::AlignCenter, strValue);

    painter.restore();
}

void DashBoardWidget::drawTitle(QPainter &painter, double minSize)
{
    painter.save();
    painter.setPen(d_ptr->titleColor);

    setupFont(painter, minSize, DashBoardWidgetPrivate::TITLE_FONT_RATIO);
    auto font = painter.font();
    font.setBold(false);
    painter.setFont(font);

    const QString strTitle = d_ptr->title.toUpper();
    const QFontMetrics fm(painter.font());
    const double textWidth = fm.horizontalAdvance(strTitle);
    const double textHeight = fm.height();
    const double centerY = minSize * DashBoardWidgetPrivate::TITLE_Y_RATIO;
    const QRectF textRect(-textWidth / 2, centerY - textHeight / 2, textWidth, textHeight);
    painter.drawText(textRect, Qt::AlignCenter, strTitle);

    painter.restore();
}
