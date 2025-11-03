#include "clockwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QTime>
#include <QTimer>
#include <QtMath>

class ClockWidget::ClockWidgetPrivate
{
public:
    explicit ClockWidgetPrivate(ClockWidget *q)
        : q_ptr(q)
    {}

    ClockWidget *q_ptr;

    // 可配置属性
    QColor borderColor = QColor(80, 80, 80);
    QColor backgroundColor = QColor(50, 50, 50);
    QColor foregroundColor = QColor(220, 220, 220);
    QColor hourColor = QColor(240, 240, 240);
    QColor minuteColor = QColor(220, 220, 220);
    QColor secondColor = QColor(255, 80, 80);
    QColor textColor = QColor(240, 240, 240);

    // 动画设置
    bool smoothAnimation = true;
    int animationDuration = 200;
    int updateInterval = 50; // 平滑动画更新间隔

    // 显示设置
    bool showSeconds = true;

    // 时间数据
    QTime currentTime;
    QTimer *updateTimer = nullptr;

    // 计算属性
    double centerX = 0;
    double centerY = 0;
    double radius = 0;

    // 常量定义
    static constexpr double BORDER_WIDTH_RATIO = 1.0 / 20.0;
    static constexpr double HOUR_HAND_LENGTH_RATIO = 0.5;
    static constexpr double MINUTE_HAND_LENGTH_RATIO = 0.7;
    static constexpr double SECOND_HAND_LENGTH_RATIO = 0.8;
    static constexpr double HAND_WIDTH_RATIO = 1.0 / 100.0;
    static constexpr double CENTER_DOT_RADIUS_RATIO = 1.0 / 40.0;
    static constexpr double SCALE_LENGTH_RATIO = 1.0 / 15.0;
    static constexpr double NUMBER_RADIUS_RATIO = 0.75;
};

ClockWidget::ClockWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ClockWidgetPrivate(this))
{
    d_ptr->currentTime = QTime::currentTime();

    // 创建更新定时器
    d_ptr->updateTimer = new QTimer(this);
    connect(d_ptr->updateTimer, &QTimer::timeout, this, &ClockWidget::onSmoothAnimationTimeout);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    startSmoothAnimation();
}

ClockWidget::~ClockWidget() = default;

auto ClockWidget::minimumSizeHint() const -> QSize
{
    return {150, 150};
}

// Border color
void ClockWidget::setBorderColor(const QColor &color)
{
    if (d_ptr->borderColor != color) {
        d_ptr->borderColor = color;
        update();
        emit borderColorChanged(color);
    }
}

auto ClockWidget::borderColor() const -> QColor
{
    return d_ptr->borderColor;
}

// Background color
void ClockWidget::setBackgroundColor(const QColor &color)
{
    if (d_ptr->backgroundColor != color) {
        d_ptr->backgroundColor = color;
        update();
        emit backgroundColorChanged(color);
    }
}

auto ClockWidget::backgroundColor() const -> QColor
{
    return d_ptr->backgroundColor;
}

// Foreground color
void ClockWidget::setForegroundColor(const QColor &color)
{
    if (d_ptr->foregroundColor != color) {
        d_ptr->foregroundColor = color;
        update();
        emit foregroundColorChanged(color);
    }
}

auto ClockWidget::foregroundColor() const -> QColor
{
    return d_ptr->foregroundColor;
}

// Hour color
void ClockWidget::setHourColor(const QColor &color)
{
    if (d_ptr->hourColor != color) {
        d_ptr->hourColor = color;
        update();
        emit hourColorChanged(color);
    }
}

auto ClockWidget::hourColor() const -> QColor
{
    return d_ptr->hourColor;
}

// Minute color
void ClockWidget::setMinuteColor(const QColor &color)
{
    if (d_ptr->minuteColor != color) {
        d_ptr->minuteColor = color;
        update();
        emit minuteColorChanged(color);
    }
}

auto ClockWidget::minuteColor() const -> QColor
{
    return d_ptr->minuteColor;
}

// Second color
void ClockWidget::setSecondColor(const QColor &color)
{
    if (d_ptr->secondColor != color) {
        d_ptr->secondColor = color;
        update();
        emit secondColorChanged(color);
    }
}

auto ClockWidget::secondColor() const -> QColor
{
    return d_ptr->secondColor;
}

// Text color
void ClockWidget::setTextColor(const QColor &color)
{
    if (d_ptr->textColor != color) {
        d_ptr->textColor = color;
        update();
        emit textColorChanged(color);
    }
}

auto ClockWidget::textColor() const -> QColor
{
    return d_ptr->textColor;
}

// Smooth animation
void ClockWidget::setSmoothAnimation(bool enabled)
{
    if (d_ptr->smoothAnimation != enabled) {
        d_ptr->smoothAnimation = enabled;

        if (enabled) {
            startSmoothAnimation();
        } else {
            stopSmoothAnimation();
        }

        update();
        emit smoothAnimationChanged(enabled);
    }
}

bool ClockWidget::smoothAnimation() const
{
    return d_ptr->smoothAnimation;
}

// Animation duration
void ClockWidget::setAnimationDuration(int duration)
{
    if (duration > 0 && d_ptr->animationDuration != duration) {
        d_ptr->animationDuration = duration;
        emit animationDurationChanged(duration);
    }
}

auto ClockWidget::animationDuration() const -> int
{
    return d_ptr->animationDuration;
}

// Show seconds
void ClockWidget::setShowSeconds(bool show)
{
    if (d_ptr->showSeconds != show) {
        d_ptr->showSeconds = show;
        update();
        emit showSecondsChanged(show);
    }
}

bool ClockWidget::showSeconds() const
{
    return d_ptr->showSeconds;
}

// Public slots
void ClockWidget::updateTime()
{
    d_ptr->currentTime = QTime::currentTime();
    update();
    emit timeUpdated(d_ptr->currentTime);
}

void ClockWidget::startSmoothAnimation()
{
    if (d_ptr->updateTimer) {
        d_ptr->updateTimer->start(d_ptr->updateInterval);
    }
}

void ClockWidget::stopSmoothAnimation()
{
    if (d_ptr->updateTimer) {
        d_ptr->updateTimer->stop();
    }
}

void ClockWidget::toggleSmoothAnimation()
{
    setSmoothAnimation(!d_ptr->smoothAnimation);
}

// Painting
void ClockWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // 更新几何计算
    updateGeometry();

    // 绘制顺序
    drawBorder(&painter);
    drawBackground(&painter);
    drawScale(&painter);
    drawScaleNumbers(&painter);
    drawHourHand(&painter);
    drawMinuteHand(&painter);

    if (d_ptr->showSeconds) {
        drawSecondHand(&painter);
    }

    drawCenterDot(&painter);
}

void ClockWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateGeometry();
}

// Private slots
void ClockWidget::onSmoothAnimationTimeout()
{
    updateTime();
}

// Private methods
void ClockWidget::updateGeometry()
{
    d_ptr->centerX = width() / 2.0;
    d_ptr->centerY = height() / 2.0;
    d_ptr->radius = qMin(d_ptr->centerX, d_ptr->centerY) * 0.9;
}

void ClockWidget::drawBorder(QPainter *painter)
{
    double borderWidth = d_ptr->radius * ClockWidgetPrivate::BORDER_WIDTH_RATIO;

    painter->save();
    painter->setPen(QPen(d_ptr->borderColor, borderWidth));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPointF(d_ptr->centerX, d_ptr->centerY),
                         d_ptr->radius - borderWidth / 2,
                         d_ptr->radius - borderWidth / 2);
    painter->restore();
}

void ClockWidget::drawBackground(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->backgroundColor);
    painter->drawEllipse(QPointF(d_ptr->centerX, d_ptr->centerY), d_ptr->radius, d_ptr->radius);
    painter->restore();
}

void ClockWidget::drawScale(QPainter *painter)
{
    painter->save();
    painter->translate(d_ptr->centerX, d_ptr->centerY);

    double scaleLength = d_ptr->radius * ClockWidgetPrivate::SCALE_LENGTH_RATIO;
    QPen scalePen(d_ptr->foregroundColor);

    for (int i = 0; i < 60; ++i) {
        if (i % 5 == 0) {
            // 小时刻度
            scalePen.setWidthF(qMax(1.0, d_ptr->radius * 0.02));
            painter->setPen(scalePen);
            painter->drawLine(0,
                              -d_ptr->radius + scaleLength * 0.5,
                              0,
                              -d_ptr->radius + scaleLength);
        } else {
            // 分钟刻度
            scalePen.setWidthF(qMax(1.0, d_ptr->radius * 0.01));
            painter->setPen(scalePen);
            painter->drawLine(0,
                              -d_ptr->radius + scaleLength * 0.7,
                              0,
                              -d_ptr->radius + scaleLength);
        }
        painter->rotate(6.0);
    }

    painter->restore();
}

void ClockWidget::drawScaleNumbers(QPainter *painter)
{
    painter->save();
    painter->setPen(d_ptr->textColor);

    QFont font = this->font();
    font.setPixelSize(qMax(10, int(d_ptr->radius * 0.1)));
    font.setBold(true);
    painter->setFont(font);

    double numberRadius = d_ptr->radius * ClockWidgetPrivate::NUMBER_RADIUS_RATIO;

    for (int i = 1; i <= 12; ++i) {
        double angle = (i - 3) * 30.0 * M_PI / 180.0;
        double x = d_ptr->centerX + numberRadius * qCos(angle);
        double y = d_ptr->centerY + numberRadius * qSin(angle);

        QString number = QString::number(i);
        QRectF textRect(x - d_ptr->radius * 0.1,
                        y - d_ptr->radius * 0.05,
                        d_ptr->radius * 0.2,
                        d_ptr->radius * 0.1);
        painter->drawText(textRect, Qt::AlignCenter, number);
    }

    painter->restore();
}

void ClockWidget::drawHourHand(QPainter *painter)
{
    painter->save();
    painter->translate(d_ptr->centerX, d_ptr->centerY);

    double hourAngle = 30.0 * (d_ptr->currentTime.hour() + d_ptr->currentTime.minute() / 60.0);
    painter->rotate(hourAngle);

    double handLength = d_ptr->radius * ClockWidgetPrivate::HOUR_HAND_LENGTH_RATIO;
    double handWidth = d_ptr->radius * ClockWidgetPrivate::HAND_WIDTH_RATIO;

    QPainterPath path;
    path.moveTo(0, handLength * 0.1);
    path.lineTo(-handWidth, 0);
    path.lineTo(0, -handLength);
    path.lineTo(handWidth, 0);
    path.closeSubpath();

    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->hourColor);
    painter->drawPath(path);

    painter->restore();
}

void ClockWidget::drawMinuteHand(QPainter *painter)
{
    painter->save();
    painter->translate(d_ptr->centerX, d_ptr->centerY);

    double minuteAngle = 6.0 * (d_ptr->currentTime.minute() + d_ptr->currentTime.second() / 60.0);
    painter->rotate(minuteAngle);

    double handLength = d_ptr->radius * ClockWidgetPrivate::MINUTE_HAND_LENGTH_RATIO;
    double handWidth = d_ptr->radius * ClockWidgetPrivate::HAND_WIDTH_RATIO * 0.8;

    QPainterPath path;
    path.moveTo(0, handLength * 0.1);
    path.lineTo(-handWidth, 0);
    path.lineTo(0, -handLength);
    path.lineTo(handWidth, 0);
    path.closeSubpath();

    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->minuteColor);
    painter->drawPath(path);

    painter->restore();
}

void ClockWidget::drawSecondHand(QPainter *painter)
{
    if (!d_ptr->showSeconds)
        return;

    painter->save();
    painter->translate(d_ptr->centerX, d_ptr->centerY);

    double secondAngle = 6.0
                         * (d_ptr->currentTime.second()
                            + (d_ptr->smoothAnimation ? d_ptr->currentTime.msec() / 1000.0 : 0));
    painter->rotate(secondAngle);

    double handLength = d_ptr->radius * ClockWidgetPrivate::SECOND_HAND_LENGTH_RATIO;
    double handWidth = d_ptr->radius * ClockWidgetPrivate::HAND_WIDTH_RATIO * 0.4;

    QPainterPath path;
    path.moveTo(0, handLength * 0.2);
    path.lineTo(-handWidth, 0);
    path.lineTo(0, -handLength);
    path.lineTo(handWidth, 0);
    path.closeSubpath();

    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->secondColor);
    painter->drawPath(path);

    painter->restore();
}

void ClockWidget::drawCenterDot(QPainter *painter)
{
    double dotRadius = d_ptr->radius * ClockWidgetPrivate::CENTER_DOT_RADIUS_RATIO;

    painter->save();
    painter->setPen(Qt::NoPen);

    // 中心圆点渐变
    QRadialGradient gradient(d_ptr->centerX, d_ptr->centerY, dotRadius);
    gradient.setColorAt(0, d_ptr->foregroundColor);
    gradient.setColorAt(1, d_ptr->borderColor);

    painter->setBrush(gradient);
    painter->drawEllipse(QPointF(d_ptr->centerX, d_ptr->centerY), dotRadius, dotRadius);

    painter->restore();
}
