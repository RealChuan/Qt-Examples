#include "dashboardwidget.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QtMath>

struct DashBoardWidget::DashBoardWidgetPrivate
{
    double maxValue = 100;
    double minValue = 0;
    double startAngle = -50;
    double endAngle = 230;
    double value = minValue;
    int scaleMajor = 10;
    int scaleMinor = 5;
    QString unit = "unit";
    QString text = "test";

    QColor arcColor = QColor(56, 61, 74);
    QColor scaleColor = QColor(4, 168, 173);
    QColor pointerColor = QColor(4, 181, 200);
    QColor textColor = QColor(144, 133, 116);
    QColor backgroundColor = Qt::transparent;

    QPropertyAnimation *animation;
};

DashBoardWidget::DashBoardWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new DashBoardWidgetPrivate)
{
    d_ptr->animation = new QPropertyAnimation(this, "value", this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(this, &DashBoardWidget::valueChanged, this, &DashBoardWidget::onStartAnimation);
}

DashBoardWidget::~DashBoardWidget() = default;

auto DashBoardWidget::sizeHint() const -> QSize
{
    return {300, 300};
}

auto DashBoardWidget::minimumSizeHint() const -> QSize
{
    return {200, 200};
}

void DashBoardWidget::setMin(const double min)
{
    d_ptr->minValue = min;
    update();
}

auto DashBoardWidget::min() const -> double
{
    return d_ptr->minValue;
}

void DashBoardWidget::setmax(const double max)
{
    d_ptr->maxValue = max;
    update();
}

auto DashBoardWidget::max() const -> double
{
    return d_ptr->maxValue;
}

void DashBoardWidget::setStartAngle(const double startAngle)
{
    d_ptr->startAngle = startAngle;
    update();
}

auto DashBoardWidget::startAngle() const -> double
{
    return d_ptr->startAngle;
}

void DashBoardWidget::setEndAngle(const double endAngle)
{
    d_ptr->endAngle = endAngle;
    update();
}

auto DashBoardWidget::endAngle() const -> double
{
    return d_ptr->endAngle;
}

void DashBoardWidget::setScaleMajor(const int scale)
{
    d_ptr->scaleMajor = scale;
    update();
}

auto DashBoardWidget::scaleMajor() const -> int
{
    return d_ptr->scaleMajor;
}

void DashBoardWidget::setScaleMinor(const int scale)
{
    d_ptr->scaleMinor = scale;
    update();
}

auto DashBoardWidget::scaleMinor() const -> int
{
    return d_ptr->scaleMinor;
}

void DashBoardWidget::setUnit(const QString &unit)
{
    d_ptr->unit = unit;
    update();
}

auto DashBoardWidget::unit() const -> QString
{
    return d_ptr->unit;
}

void DashBoardWidget::setText(const QString &text)
{
    d_ptr->text = text;
    update();
}

auto DashBoardWidget::text() const -> QString
{
    return d_ptr->text;
}

void DashBoardWidget::setArcColor(const QColor &color)
{
    d_ptr->arcColor = color;
    update();
}

auto DashBoardWidget::arcColor() const -> QColor
{
    return d_ptr->arcColor;
}

void DashBoardWidget::setScaleColor(const QColor &color)
{
    d_ptr->scaleColor = color;
    update();
}

auto DashBoardWidget::scaleColor() const -> QColor
{
    return d_ptr->scaleColor;
}

void DashBoardWidget::setPointerColor(const QColor &color)
{
    d_ptr->pointerColor = color;
    update();
}

auto DashBoardWidget::pointerColor() const -> QColor
{
    return d_ptr->pointerColor;
}

void DashBoardWidget::setTextColor(const QColor &color)
{
    d_ptr->textColor = color;
    update();
}

auto DashBoardWidget::textColor() const -> QColor
{
    return d_ptr->textColor;
}

void DashBoardWidget::setBackgroundColor(const QColor &color)
{
    d_ptr->backgroundColor = color;
    update();
}

auto DashBoardWidget::backgroundColor() const -> QColor
{
    return d_ptr->backgroundColor;
}

void DashBoardWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // 背景
    if (d_ptr->backgroundColor != Qt::transparent) {
        painter.setPen(Qt::NoPen);
        painter.fillRect(rect(), d_ptr->backgroundColor);
    }

    // 平移中心
    painter.translate(width() / 2, height() / 2);

    // 圆环
    drawArc(&painter);

    // 刻度线
    drawScale(&painter);

    // 刻度值
    drawScaleNum(&painter);

    // 指示器
    drawPointer(&painter);

    // 当前值
    drawValue(&painter);
}

void DashBoardWidget::onStartAnimation(const double value)
{
    if (value < d_ptr->minValue || value > d_ptr->maxValue || value == d_ptr->value) {
        return;
}
    
    int start = d_ptr->value;
    int end = value;
    
    d_ptr->animation->setStartValue(start);
    d_ptr->animation->setEndValue(end);
    d_ptr->animation->start();
}

auto DashBoardWidget::value() const -> double
{
    return d_ptr->value;
}

void DashBoardWidget::setValue(const double value)
{
    d_ptr->value = value;
    update();
}

void DashBoardWidget::drawArc(QPainter *painter)
{
    double min = qMin(width(), height());
    double arcWidth = min / 15.0;
    double radius = min / 3 - arcWidth;
    QRectF rect = QRectF(-radius, -radius, radius * 2, radius * 2);
    QPen pen;
    pen.setWidthF(arcWidth);
    pen.setCapStyle(Qt::FlatCap);

    // 圆弧背景
    double angle = d_ptr->endAngle - d_ptr->startAngle;
    pen.setColor(d_ptr->arcColor);
    painter->setPen(pen);
    painter->drawArc(rect, d_ptr->startAngle * 16, angle * 16);
}

void DashBoardWidget::drawScale(QPainter *painter)
{
    painter->save();
    
    painter->rotate(270 - d_ptr->endAngle);
    int steps = (d_ptr->scaleMajor * d_ptr->scaleMinor);
    double angleStep = (d_ptr->endAngle - d_ptr->startAngle) / steps;
    double min = qMin(width(), height());
    double radius = min / 3;
    
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

void DashBoardWidget::drawScaleNum(QPainter *painter)
{
    painter->save();
    painter->setPen(d_ptr->scaleColor);
    double min = qMin(width(), height());
    double radius = min / 2.4;
    QFont font("Microsoft YaHei", min / 25);
    painter->setFont(font);
    
    double startRad = d_ptr->endAngle * (M_PI / 180);
    double deltaRad = (d_ptr->endAngle - d_ptr->startAngle) * (M_PI / 180) / d_ptr->scaleMajor;

    QFontMetrics fontMetrics(font);
    for (int i = 0; i <= d_ptr->scaleMajor; i++) {
        double sina = qSin(startRad - i * deltaRad);
        double cosa = qCos(startRad - i * deltaRad);
        double value = 1.0 * i * ((d_ptr->maxValue - d_ptr->minValue) / d_ptr->scaleMajor) + d_ptr->minValue;

        QString strValue = QString("%1").arg(value, 0, 'f', 2);
        double textWidth = fontMetrics.horizontalAdvance(strValue);
        double textHeight = fontMetrics.height();
        int x = radius * cosa - textWidth / 2;
        int y = -radius * sina + textHeight / 4;
        painter->drawText(x, y, strValue);
    }

    painter->restore();
}

void DashBoardWidget::drawPointer(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->pointerColor);

    double radius = qMin(width(), height()) / 3.0;

    QPolygonF pts;
    pts << QPointF(-5, 0) << QPointF(0, -8) << QPointF(5, 0) << QPointF(0, radius);
    
    painter->rotate(270 - d_ptr->endAngle);
    double degRotate = (d_ptr->endAngle - d_ptr->startAngle) / (d_ptr->maxValue - d_ptr->minValue)
                       * (d_ptr->value - d_ptr->minValue);
    painter->rotate(degRotate);
    painter->drawConvexPolygon(pts);

    painter->restore();
}

void DashBoardWidget::drawValue(QPainter *painter)
{
    painter->save();
    painter->setPen(d_ptr->textColor);

    double min = qMin(width(), height());
    double radius = min / 2.0 - min / 4.8;
    QFont font("Microsoft YaHei", min / 25);
    painter->setFont(font);
    
    QString strValue = QString("%1").arg(d_ptr->value, 0, 'f', 2);
    strValue = QString("%1 %2").arg(strValue, d_ptr->unit);
    QRectF valueRect(-radius, radius / 2.5, radius * 2, radius / 3.5);
    painter->drawText(valueRect, Qt::AlignCenter, strValue);

    QRectF textRect(-radius, radius / 1.5, radius * 2, radius / 2.5);
    //font.setPixelSize(12);
    painter->setFont(font);
    painter->drawText(textRect, Qt::AlignCenter, d_ptr->text);

    painter->restore();
}
