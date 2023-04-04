#include "batterywidget.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QTextItem>
#include <QWheelEvent>
#include <QtMath>

struct BatteryWidget::BatteryWidgetPrivate
{
    QColor borderColor = QColor(80, 80, 80);
    QColor powerColor = QColor(65, 205, 82);
    QColor alarmColor = QColor(250, 118, 113);
    int alarmValue = 20;
    int value = 0;
    QPropertyAnimation *animation;
};

BatteryWidget::BatteryWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new BatteryWidgetPrivate)
{
    d_ptr->animation = new QPropertyAnimation(this, "value", this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(this, &BatteryWidget::valueChanged, this, &BatteryWidget::onStartAnimation);
}

BatteryWidget::~BatteryWidget() = default;

void BatteryWidget::setBorderColor(const QColor &color)
{
    d_ptr->borderColor = color;
    update();
}

auto BatteryWidget::borderColor() const -> QColor
{
    return d_ptr->borderColor;
}

void BatteryWidget::setPowerColor(const QColor &color)
{
    d_ptr->powerColor = color;
    update();
}

auto BatteryWidget::powerColor() const -> QColor
{
    return d_ptr->powerColor;
}

void BatteryWidget::setAlarmColor(const QColor &color)
{
    d_ptr->alarmColor = color;
    update();
}

auto BatteryWidget::alarmColor() const -> QColor
{
    return d_ptr->alarmColor;
}

auto BatteryWidget::sizeHint() const -> QSize
{
    return {150, 80};
}

auto BatteryWidget::minimumSizeHint() const -> QSize
{
    return {80, 45};
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
    if (value == d_ptr->value) {
        return;
    }

    int start = d_ptr->value;
    int end = value;

    d_ptr->animation->setStartValue(start);
    d_ptr->animation->setEndValue(end);
    d_ptr->animation->start();
}

auto BatteryWidget::value() const -> int
{
    return d_ptr->value;
}

void BatteryWidget::setValue(int value)
{
    Q_ASSERT(value >= 0 && value <= 100);

    if (value == d_ptr->value) {
        return;
    }

    d_ptr->value = value;

    update();
}

void BatteryWidget::drawBorder(QPainter *painter, const QRectF &batteryRect, const double linew)
{
    painter->setPen(QPen(d_ptr->borderColor, linew));
    painter->setBrush(Qt::NoBrush);
    double borderRadius = batteryRect.height() / 30;
    painter->drawRoundedRect(batteryRect, borderRadius, borderRadius);
}

void BatteryWidget::drawPower(QPainter *painter, const QRectF &batteryRect, const double linew)
{
    QColor powerColoer = d_ptr->value > d_ptr->alarmValue ? d_ptr->powerColor : d_ptr->alarmColor;
    double margin = qMin(width(), height()) / 50.0;
    margin = qMax(margin, linew);
    qreal unit = (batteryRect.width() - (margin * 2)) / 100;
    QPointF topLeft(batteryRect.topLeft().x() + margin, batteryRect.topLeft().y() + margin);
    QPointF bottomRight(d_ptr->value * unit + margin + 5, batteryRect.bottomRight().y() - margin);
    QRectF rect(topLeft, bottomRight);
    double bgRadius = rect.height() / 30;
    painter->setPen(Qt::NoPen);
    painter->setBrush(powerColoer);
    painter->drawRoundedRect(rect, bgRadius, bgRadius);
}

void BatteryWidget::drawValue(QPainter *painter, const QRectF &batteryRect)
{
    QColor fontColoer = d_ptr->value > d_ptr->alarmValue ? QColor(64, 65, 66) : d_ptr->alarmColor;
    QString text = QString("%1%").arg(d_ptr->value);
    QFont font("Microsoft YaHei", batteryRect.width() / 5);
    font.setLetterSpacing(QFont::AbsoluteSpacing, batteryRect.width() / 25);
    painter->setFont(font);
    painter->setPen(fontColoer);
    painter->setBrush(Qt::NoBrush);
    painter->drawText(batteryRect, Qt::AlignCenter, text);
}

void BatteryWidget::drawHeader(QPainter *painter, const QRectF &batteryRect)
{
    QPointF headRectTopLeft(batteryRect.topRight().x(), height() / 3);
    QPointF headRectBottomRight(width(), height() - height() / 3);
    QRectF headRect(headRectTopLeft, headRectBottomRight);
    double headRadius = headRect.height() / 30;
    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->borderColor);
    painter->drawRoundedRect(headRect, headRadius, headRadius);
}
