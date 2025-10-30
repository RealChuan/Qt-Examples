#include "batterywidget.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>

class BatteryWidget::BatteryWidgetPrivate
{
public:
    explicit BatteryWidgetPrivate(BatteryWidget *q)
        : q_ptr(q)
    {}

    BatteryWidget *q_ptr;

    // 可配置属性
    QColor borderColor = QColor(80, 80, 80);
    QColor powerColor = QColor(65, 205, 82);
    QColor alarmColor = QColor(250, 118, 113);
    int alarmValue = 20;
    int value = 0;
    int animationDuration = 500;

    // 状态标志
    bool isInAlarmState = false;
    bool wasInAlarmState = false;
    bool charging = false;

    // 动画
    QPropertyAnimation *animation = nullptr;
    int targetValue = 0; // 动画目标值

    // 常量定义
    static constexpr double BORDER_MARGIN_RATIO = 0.03;
    static constexpr double HEAD_WIDTH_RATIO = 1.0 / 11.0;
    static constexpr double CORNER_RADIUS_RATIO = 1.0 / 30.0;
    static constexpr double POWER_MARGIN_RATIO = 0.02;
    static constexpr double FONT_SIZE_RATIO = 0.5;
    static constexpr double CHARGING_SYMBOL_FONT_RATIO = 0.5; // 充电符号字体比例
};

BatteryWidget::BatteryWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new BatteryWidgetPrivate(this))
{
    d_ptr->animation = new QPropertyAnimation(this, "value", this);
    d_ptr->animation->setDuration(d_ptr->animationDuration);
    d_ptr->animation->setEasingCurve(QEasingCurve::InOutQuad);

    // 连接内部信号
    connect(d_ptr->animation,
            &QPropertyAnimation::finished,
            this,
            &BatteryWidget::onAnimationFinished);
    connect(this, &BatteryWidget::valueChanged, this, &BatteryWidget::onValueChanged);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 初始报警状态检查
    d_ptr->isInAlarmState = (d_ptr->value <= d_ptr->alarmValue);
    d_ptr->wasInAlarmState = d_ptr->isInAlarmState;
}

BatteryWidget::~BatteryWidget() = default;

// Size hints
auto BatteryWidget::sizeHint() const -> QSize
{
    return {150, 80};
}

auto BatteryWidget::minimumSizeHint() const -> QSize
{
    return {80, 45};
}

// Border color
void BatteryWidget::setBorderColor(const QColor &color)
{
    if (d_ptr->borderColor != color) {
        d_ptr->borderColor = color;
        update();
    }
}

auto BatteryWidget::borderColor() const -> QColor
{
    return d_ptr->borderColor;
}

// Power color
void BatteryWidget::setPowerColor(const QColor &color)
{
    if (d_ptr->powerColor != color) {
        d_ptr->powerColor = color;
        update();
    }
}

auto BatteryWidget::powerColor() const -> QColor
{
    return d_ptr->powerColor;
}

// Alarm color
void BatteryWidget::setAlarmColor(const QColor &color)
{
    if (d_ptr->alarmColor != color) {
        d_ptr->alarmColor = color;
        update();
    }
}

auto BatteryWidget::alarmColor() const -> QColor
{
    return d_ptr->alarmColor;
}

// Alarm value
void BatteryWidget::setAlarmValue(int value)
{
    value = qBound(0, value, 100);
    if (d_ptr->alarmValue != value) {
        d_ptr->alarmValue = value;
        checkAlarmState(); // 重新检查报警状态
        update();
    }
}

auto BatteryWidget::alarmValue() const -> int
{
    return d_ptr->alarmValue;
}

// Value
auto BatteryWidget::value() const -> int
{
    return d_ptr->value;
}

void BatteryWidget::setValue(int value)
{
    value = qBound(0, value, 100);

    if (value == d_ptr->value) {
        return;
    }

    d_ptr->value = value;
    update();

    // 只有在非动画过程中或者动画完成时才发出valueChanged信号
    if (!isAnimating() || value == d_ptr->targetValue) {
        emit valueChanged(value);
    }
}

void BatteryWidget::setValueAnimated(int value)
{
    value = qBound(0, value, 100);

    if (value == d_ptr->value) {
        return;
    }

    // 记录目标值
    d_ptr->targetValue = value;

    // 发出动画开始信号
    emit animationStarted(d_ptr->value, value);

    startAnimation(value);
}

// Animation duration
void BatteryWidget::setAnimationDuration(int duration)
{
    if (duration >= 0 && d_ptr->animationDuration != duration) {
        d_ptr->animationDuration = duration;
        d_ptr->animation->setDuration(duration);
    }
}

auto BatteryWidget::animationDuration() const -> int
{
    return d_ptr->animationDuration;
}

// Animation state
bool BatteryWidget::isAnimating() const
{
    return d_ptr->animation && d_ptr->animation->state() == QPropertyAnimation::Running;
}

// Charging state
void BatteryWidget::setCharging(bool charging)
{
    if (d_ptr->charging != charging) {
        d_ptr->charging = charging;
        update();
        emit chargingChanged(charging);
    }
}

bool BatteryWidget::isCharging() const
{
    return d_ptr->charging;
}

// Public slots implementation
void BatteryWidget::increaseValue(int increment)
{
    if (increment <= 0)
        return;

    int newValue = qMin(100, d_ptr->value + increment);
    if (newValue != d_ptr->value) {
        setValue(newValue);
        emit valueIncreased(newValue);
    }
}

void BatteryWidget::decreaseValue(int decrement)
{
    if (decrement <= 0)
        return;

    int newValue = qMax(0, d_ptr->value - decrement);
    if (newValue != d_ptr->value) {
        setValue(newValue);
        emit valueDecreased(newValue);
    }
}

void BatteryWidget::reset()
{
    if (d_ptr->value != 0) {
        setValue(0);
        emit valueReset();
    }
}

// Painting
void BatteryWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing
                           | QPainter::SmoothPixmapTransform);

    // 计算线宽
    auto linew = 2 * painter.pen().widthF() / painter.transform().m11();

    // 计算电池主体和头部尺寸
    qreal headWidth = width() * BatteryWidgetPrivate::HEAD_WIDTH_RATIO;
    qreal batteryWidth = width() - headWidth;

    // 计算边框边距
    qreal borderMargin = qMin(width(), height()) * BatteryWidgetPrivate::BORDER_MARGIN_RATIO;
    borderMargin = qMax(borderMargin, linew);

    auto batteryRect = QRectF(QPointF(borderMargin, borderMargin),
                              QPointF(batteryWidth - borderMargin, height() - borderMargin));

    // 绘制顺序
    drawBorder(&painter, batteryRect, linew);
    drawPower(&painter, batteryRect, linew);

    // 如果正在充电，绘制闪电符号作为背景
    if (d_ptr->charging) {
        // 数值显示在左侧2/3区域
        QRectF valueRect = batteryRect;
        valueRect.setWidth(batteryRect.width() * 2.0 / 3.0);
        drawValue(&painter, valueRect);

        // 闪电显示在右侧1/3区域
        QRectF chargingRect = batteryRect;
        chargingRect.setLeft(batteryRect.left() + batteryRect.width() * 2.0 / 3.0);
        drawChargingSymbol(&painter, chargingRect);
    } else {
        // 非充电状态正常显示数值
        drawValue(&painter, batteryRect);
    }

    drawHeader(&painter, batteryRect);
}

void BatteryWidget::startAnimation(int targetValue)
{
    if (targetValue == d_ptr->value || !d_ptr->animation) {
        return;
    }

    // 如果动画正在运行，停止它
    if (d_ptr->animation->state() == QPropertyAnimation::Running) {
        d_ptr->animation->stop();
    }

    d_ptr->animation->setStartValue(d_ptr->value);
    d_ptr->animation->setEndValue(targetValue);
    d_ptr->animation->start();
}

void BatteryWidget::onAnimationFinished()
{
    // 动画完成时发出完成信号
    emit animationFinished(d_ptr->value);

    // 确保最终值发出valueChanged信号
    if (d_ptr->value == d_ptr->targetValue) {
        emit valueChanged(d_ptr->value);
    }
}

void BatteryWidget::onValueChanged(int value)
{
    Q_UNUSED(value)
    checkAlarmState();
}

void BatteryWidget::checkAlarmState()
{
    d_ptr->isInAlarmState = (d_ptr->value <= d_ptr->alarmValue);

    // 如果报警状态发生变化，发出信号
    if (d_ptr->isInAlarmState != d_ptr->wasInAlarmState) {
        emit alarmStateChanged(d_ptr->isInAlarmState);
        d_ptr->wasInAlarmState = d_ptr->isInAlarmState;
    }
}

void BatteryWidget::drawBorder(QPainter *painter, const QRectF &batteryRect, const qreal linew)
{
    painter->setPen(QPen(d_ptr->borderColor, linew));
    painter->setBrush(Qt::NoBrush);
    qreal borderRadius = batteryRect.height() * BatteryWidgetPrivate::CORNER_RADIUS_RATIO;
    painter->drawRoundedRect(batteryRect, borderRadius, borderRadius);
}

void BatteryWidget::drawPower(QPainter *painter, const QRectF &batteryRect, const qreal linew)
{
    if (d_ptr->value <= 0) {
        return;
    }

    // 确定电量颜色
    auto powerColor = d_ptr->isInAlarmState ? d_ptr->alarmColor : d_ptr->powerColor;

    // 计算电量区域边距
    qreal powerMargin = qMin(width(), height()) * BatteryWidgetPrivate::POWER_MARGIN_RATIO;
    powerMargin = qMax(powerMargin, linew);

    // 计算可用宽度和电量宽度
    qreal availableWidth = batteryRect.width() - 2 * powerMargin;
    qreal powerWidth = availableWidth * d_ptr->value / 100.0;

    // 创建电量矩形
    QPointF topLeft(batteryRect.left() + powerMargin, batteryRect.top() + powerMargin);
    QPointF bottomRight(batteryRect.left() + powerMargin + powerWidth,
                        batteryRect.bottom() - powerMargin);
    QRectF powerRect(topLeft, bottomRight);

    // 绘制电量
    qreal powerRadius = powerRect.height() * BatteryWidgetPrivate::CORNER_RADIUS_RATIO;
    painter->setPen(Qt::NoPen);
    painter->setBrush(powerColor);
    painter->drawRoundedRect(powerRect, powerRadius, powerRadius);
}

void BatteryWidget::drawValue(QPainter *painter, const QRectF &batteryRect)
{
    if (d_ptr->value == 0 && !d_ptr->charging) {
        return;
    }

    // 设置字体
    QFont font = this->font();
    qreal fontSize = batteryRect.height() * BatteryWidgetPrivate::FONT_SIZE_RATIO;
    fontSize = qMin(fontSize, batteryRect.height() * 0.6); // 限制最大字体大小
    font.setPixelSize(qRound(fontSize));
    font.setWeight(QFont::Bold);

    QString text = QString("%1%").arg(d_ptr->value);

    // 确定文字颜色 - 充电状态下使用深色文字以确保可读性
    QColor textColor;
    if (d_ptr->charging) {
        // 充电状态下使用深色文字以确保在浅色闪电上可读
        textColor = QColor(30, 30, 30);
    } else {
        textColor = d_ptr->isInAlarmState ? d_ptr->alarmColor : QColor(64, 65, 66);
    }

    painter->setFont(font);
    painter->setPen(textColor);
    painter->setBrush(Qt::NoBrush);
    painter->drawText(batteryRect, Qt::AlignCenter, text);
}

void BatteryWidget::drawHeader(QPainter *painter, const QRectF &batteryRect)
{
    qreal headHeight = batteryRect.height() / 3.0;
    qreal headVerticalMargin = (height() - headHeight) / 2.0;

    QPointF headRectTopLeft(batteryRect.topRight().x(), headVerticalMargin);
    QPointF headRectBottomRight(width() - BatteryWidgetPrivate::BORDER_MARGIN_RATIO * width(),
                                height() - headVerticalMargin);
    QRectF headRect(headRectTopLeft, headRectBottomRight);

    qreal headRadius = headRect.height() * BatteryWidgetPrivate::CORNER_RADIUS_RATIO;
    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->borderColor);
    painter->drawRoundedRect(headRect, headRadius, headRadius);
}

void BatteryWidget::drawChargingSymbol(QPainter *painter, const QRectF &batteryRect)
{
    static QString text = "⚡";
    auto font = painter->font();
    font.setPixelSize(batteryRect.height() * BatteryWidgetPrivate::CHARGING_SYMBOL_FONT_RATIO);

    painter->setFont(font);
    painter->setPen(Qt::white);
    painter->drawText(batteryRect, Qt::AlignCenter, text);
}
