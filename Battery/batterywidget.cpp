#include "batterywidget.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QTextItem>
#include <QWheelEvent>
#include <QtMath>

struct BatteryWidgetPrivate{
    QColor borderColor = QColor(80, 80, 80);
    QColor powerColor = QColor(65, 205, 82);
    QColor alarmColor = QColor(250, 118, 113);
    int alarmValue = 20;
    int value = 0;
    QPropertyAnimation *animation;
};

BatteryWidget::BatteryWidget(QWidget *parent)
    : QWidget(parent)
    , d(new BatteryWidgetPrivate)
{
    d->animation = new QPropertyAnimation(this, "value", this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(this, &BatteryWidget::valueChanged, this, &BatteryWidget::onStartAnimation);
}

BatteryWidget::~BatteryWidget()
{
}

void BatteryWidget::setBorderColor(const QColor &color)
{
    d->borderColor = color;
    update();
}

QColor BatteryWidget::borderColor() const
{
    return d->borderColor;
}

void BatteryWidget::setPowerColor(const QColor &color)
{
    d->powerColor = color;
    update();
}

QColor BatteryWidget::powerColor() const
{
    return d->powerColor;
}

void BatteryWidget::setAlarmColor(const QColor &color)
{
    d->alarmColor = color;
    update();
}

QColor BatteryWidget::alarmColor() const
{
    return d->alarmColor;
}

QSize BatteryWidget::sizeHint() const
{
    return QSize(150, 80);
}

QSize BatteryWidget::minimumSizeHint() const
{
    return QSize(80, 45);
}

void BatteryWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    double linew = 2 * painter.pen().widthF() / painter.transform().m11();

    qreal headWidth = width() / 11;
    qreal batteryWidth = width() - headWidth;
    QRectF batteryRect = QRectF(QPointF(5, 5), QPointF(batteryWidth, height() - 5));

    // 边框
    drawBorder(&painter, batteryRect, linew);

    // 电量
    drawPower(&painter, batteryRect, linew);

    // 数值
    drawValue(&painter, batteryRect);

    // 头部
    drawHeader(&painter, batteryRect);
}

void BatteryWidget::onStartAnimation(const int value)
{
    if(value == d->value)
        return;

    int start = d->value;
    int end = value;

    d->animation->setStartValue(start);
    d->animation->setEndValue(end);
    d->animation->start();
}

int BatteryWidget::value() const
{
    return d->value;
}

void BatteryWidget::setValue(const int value)
{
    if(value == d->value)
        return;

    if(value < 0)
        d->value = 0;
    else if(value > 100)
        d->value = 100;
    else
        d->value = value;

    update();
}

void BatteryWidget::drawBorder(QPainter *painter, const QRectF &batteryRect, const double linew)
{
    painter->setPen(QPen(d->borderColor, linew));
    painter->setBrush(Qt::NoBrush);
    double borderRadius = batteryRect.height() / 30;
    painter->drawRoundedRect(batteryRect, borderRadius, borderRadius);
}

void BatteryWidget::drawPower(QPainter *painter, const QRectF &batteryRect, const double linew)
{
    QColor powerColoer = d->value > d->alarmValue ?
                d->powerColor : d->alarmColor;
    double margin = qMin(width(), height()) / 50.0;
    margin = qMax(margin, linew);
    qreal unit = (batteryRect.width() - (margin * 2)) / 100;
    QPointF topLeft(batteryRect.topLeft().x() + margin,
                    batteryRect.topLeft().y() + margin);
    QPointF bottomRight(d->value * unit + margin + 5,
                        batteryRect.bottomRight().y() - margin);
    QRectF rect(topLeft, bottomRight);
    double bgRadius = rect.height() / 30;
    painter->setPen(Qt::NoPen);
    painter->setBrush(powerColoer);
    painter->drawRoundedRect(rect, bgRadius, bgRadius);
}

void BatteryWidget::drawValue(QPainter *painter, const QRectF &batteryRect)
{
    QColor fontColoer = d->value > d->alarmValue ?
                QColor(64, 65, 66) : d->alarmColor;
    QString text = QString("%1%").arg(d->value);
    QFont font("Microsoft YaHei", batteryRect.width() / 5);
    font.setLetterSpacing(QFont::AbsoluteSpacing, batteryRect.width() / 25);
    painter->setFont(font);
    painter->setPen(fontColoer);
    painter->setBrush(Qt::NoBrush);
    painter->drawText(batteryRect, Qt::AlignHCenter | Qt::AlignVCenter, text);
}

void BatteryWidget::drawHeader(QPainter *painter, const QRectF &batteryRect)
{
    QPointF headRectTopLeft(batteryRect.topRight().x(), height() / 3);
    QPointF headRectBottomRight(width(), height() - height() / 3);
    QRectF headRect(headRectTopLeft, headRectBottomRight);
    double headRadius = headRect.height() / 30;
    painter->setPen(Qt::NoPen);
    painter->setBrush(d->borderColor);
    painter->drawRoundedRect(headRect, headRadius, headRadius);
}
