#include "dashboardwidget.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QtMath>

struct DashBoardWidgetPrivate{
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
    , d(new DashBoardWidgetPrivate)
{
    d->animation = new QPropertyAnimation(this, "value", this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(this, &DashBoardWidget::valueChanged, this, &DashBoardWidget::onStartAnimation);
}

DashBoardWidget::~DashBoardWidget()
{
}

QSize DashBoardWidget::sizeHint() const
{
    return QSize(300, 300);
}

QSize DashBoardWidget::minimumSizeHint() const
{
    return QSize(200, 200);
}

void DashBoardWidget::setMin(const double min)
{
    d->minValue = min;
    update();
}

double DashBoardWidget::min() const
{
    return d->minValue;
}

void DashBoardWidget::setmax(const double max)
{
    d->maxValue = max;
    update();
}

double DashBoardWidget::max() const
{
    return d->maxValue;
}

void DashBoardWidget::setStartAngle(const double startAngle)
{
    d->startAngle = startAngle;
    update();
}

double DashBoardWidget::startAngle() const
{
    return d->startAngle;
}

void DashBoardWidget::setEndAngle(const double endAngle)
{
    d->endAngle = endAngle;
    update();
}

double DashBoardWidget::endAngle() const
{
    return d->endAngle;
}

void DashBoardWidget::setScaleMajor(const int scale)
{
    d->scaleMajor = scale;
    update();
}

int DashBoardWidget::scaleMajor() const
{
    return d->scaleMajor;
}

void DashBoardWidget::setScaleMinor(const int scale)
{
    d->scaleMinor = scale;
    update();
}

int DashBoardWidget::scaleMinor() const
{
    return d->scaleMinor;
}

void DashBoardWidget::setUnit(const QString &unit)
{
    d->unit = unit;
    update();
}

QString DashBoardWidget::unit() const
{
    return d->unit;
}

void DashBoardWidget::setText(const QString &text)
{
    d->text = text;
    update();
}

QString DashBoardWidget::text() const
{
    return d->text;
}

void DashBoardWidget::setArcColor(const QColor &color)
{
    d->arcColor = color;
    update();
}

QColor DashBoardWidget::arcColor() const
{
    return d->arcColor;
}

void DashBoardWidget::setScaleColor(const QColor &color)
{
    d->scaleColor = color;
    update();
}

QColor DashBoardWidget::scaleColor() const
{
    return d->scaleColor;
}

void DashBoardWidget::setPointerColor(const QColor &color)
{
    d->pointerColor = color;
    update();
}

QColor DashBoardWidget::pointerColor() const
{
    return d->pointerColor;
}

void DashBoardWidget::setTextColor(const QColor &color)
{
    d->textColor = color;
    update();
}

QColor DashBoardWidget::textColor() const
{
    return d->textColor;
}

void DashBoardWidget::setBackgroundColor(const QColor &color)
{
    d->backgroundColor = color;
    update();
}

QColor DashBoardWidget::backgroundColor() const
{
    return d->backgroundColor;
}

void DashBoardWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // 背景
    if (d->backgroundColor != Qt::transparent) {
        painter.setPen(Qt::NoPen);
        painter.fillRect(rect(), d->backgroundColor);
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
    if(value < d->minValue
            || value > d->maxValue
            || value == d->value)
        return;

    int start = d->value;
    int end = value;

    d->animation->setStartValue(start);
    d->animation->setEndValue(end);
    d->animation->start();
}

double DashBoardWidget::value() const
{
    return d->value;
}

void DashBoardWidget::setValue(const double value)
{
    d->value = value;
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
    double angle = d->endAngle - d->startAngle;
    pen.setColor(d->arcColor);
    painter->setPen(pen);
    painter->drawArc(rect, d->startAngle * 16, angle * 16);
}

void DashBoardWidget::drawScale(QPainter *painter)
{
    painter->save();

    painter->rotate(270 - d->endAngle);
    int steps = (d->scaleMajor * d->scaleMinor);
    double angleStep = (d->endAngle - d->startAngle) / steps;
    double min = qMin(width(), height());
    double radius = min / 3;

    QPen pen(d->scaleColor);
    pen.setCapStyle(Qt::RoundCap);

    for (int i = 0; i <= steps; i++) {
        if (i % d->scaleMinor == 0) {
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
    painter->setPen(d->scaleColor);
    double min = qMin(width(), height());
    double radius = min / 2.4;
    QFont font("Microsoft YaHei", min / 25);
    painter->setFont(font);

    double startRad = d->endAngle * (M_PI / 180);
    double deltaRad = (d->endAngle - d->startAngle) * (M_PI / 180) / d->scaleMajor;

    QFontMetrics fontMetrics(font);
    for (int i = 0; i <= d->scaleMajor; i++) {
        double sina = qSin(startRad - i * deltaRad);
        double cosa = qCos(startRad - i * deltaRad);
        double value = 1.0 * i * ((d->maxValue - d->minValue) / d->scaleMajor)
                + d->minValue;

        QString strValue = QString("%1").arg(value, 0, 'f', 2);
        double textWidth = fontMetrics.width(strValue);
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
    painter->setBrush(d->pointerColor);

    double radius = qMin(width(), height()) / 3.0;

    QPolygonF pts;
    pts << QPointF(-5, 0) << QPointF(0, -8)
        << QPointF(5, 0) << QPointF(0, radius);

    painter->rotate(270 - d->endAngle);
    double degRotate = (d->endAngle - d->startAngle) / (d->maxValue - d->minValue) * (d->value - d->minValue);
    painter->rotate(degRotate);
    painter->drawConvexPolygon(pts);

    painter->restore();
}

void DashBoardWidget::drawValue(QPainter *painter)
{
    painter->save();
    painter->setPen(d->textColor);

    double min = qMin(width(), height());
    double radius = min / 2.0 - min / 4.8;
    QFont font("Microsoft YaHei", min / 25);
    painter->setFont(font);

    QString strValue = QString("%1").arg(d->value, 0, 'f', 2);
    strValue = QString("%1 %2").arg(strValue).arg(d->unit);
    QRectF valueRect(-radius, radius / 2.5, radius * 2, radius / 3.5);
    painter->drawText(valueRect, Qt::AlignCenter, strValue);

    QRectF textRect(-radius, radius / 1.5, radius * 2, radius / 2.5);
    //font.setPixelSize(12);
    painter->setFont(font);
    painter->drawText(textRect, Qt::AlignCenter, d->text);

    painter->restore();
}
