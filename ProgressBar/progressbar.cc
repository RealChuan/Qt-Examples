#include "progressbar.hpp"

#include <QPainter>
#include <QPropertyAnimation>

struct ProgressBar::ProgressBarPrivate
{
    double minValue = 0;
    double maxValue = 100;
    double value = 0;
    double radius = -1;
    bool autoRadius = true;

    QColor chunkColor = QColor(QLatin1String("#4da1ff"));
    QColor textColor = QColor(64, 65, 66);
    QColor baseColor = QColor(179, 179, 179);
    QColor backgroundColor = Qt::transparent;

    bool percent = true;

    QPropertyAnimation *animation;
};

ProgressBar::ProgressBar(QWidget *parent)
    : QWidget{parent}
    , d_ptr(new ProgressBarPrivate)
{
    d_ptr->animation = new QPropertyAnimation(this, "value", this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    connect(this, &ProgressBar::valueChanged, this, &ProgressBar::onStartAnimation);
}

ProgressBar::~ProgressBar() = default;

auto ProgressBar::sizeHint() const -> QSize
{
    return {165, 15};
}

auto ProgressBar::minimumSizeHint() const -> QSize
{
    return {100, 15};
}

void ProgressBar::setValue(const double value)
{
    d_ptr->value = value;
    update();
}

auto ProgressBar::value() const -> double
{
    return d_ptr->value;
}

void ProgressBar::setPercent(const bool percent)
{
    d_ptr->percent = percent;
    update();
}

auto ProgressBar::percent() const -> bool
{
    return d_ptr->percent;
}

void ProgressBar::setMin(const double min)
{
    d_ptr->minValue = min;
    update();
}

auto ProgressBar::min() const -> double
{
    return d_ptr->minValue;
}

void ProgressBar::setmax(const double max)
{
    d_ptr->maxValue = max;
    update();
}

auto ProgressBar::max() const -> double
{
    return d_ptr->maxValue;
}

void ProgressBar::setRadius(const double radius)
{
    d_ptr->radius = radius;
    update();
}

auto ProgressBar::radius() const -> double
{
    return d_ptr->radius;
}

void ProgressBar::setAutoRadius(bool autoRadius)
{
    d_ptr->autoRadius = autoRadius;
    update();
}

auto ProgressBar::autoRadius() const -> double
{
    return d_ptr->autoRadius;
}

void ProgressBar::setChunkColor(const QColor &color)
{
    d_ptr->chunkColor = color;
    update();
}

auto ProgressBar::chunkColor() const -> QColor
{
    return d_ptr->chunkColor;
}

void ProgressBar::setTextColor(const QColor &color)
{
    d_ptr->textColor = color;
    update();
}

auto ProgressBar::textColor() const -> QColor
{
    return d_ptr->textColor;
}

void ProgressBar::setBaseColor(const QColor &color)
{
    d_ptr->baseColor = color;
    update();
}

auto ProgressBar::baseColor() const -> QColor
{
    return d_ptr->baseColor;
}

void ProgressBar::setBackgroundColor(const QColor &color)
{
    d_ptr->backgroundColor = color;
    update();
}

auto ProgressBar::backgroundColor() const -> QColor
{
    return d_ptr->backgroundColor;
}

void ProgressBar::onStartAnimation(const double value)
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

void ProgressBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // 背景
    if (d_ptr->backgroundColor != Qt::transparent) {
        painter.setPen(Qt::NoPen);
        painter.fillRect(rect(), d_ptr->backgroundColor);
    }

    drawProgressBar(&painter);

    drawText(&painter);
}

void ProgressBar::drawProgressBar(QPainter *painter)
{
    QRectF rect = this->rect();
    if (d_ptr->radius <= 0 && d_ptr->autoRadius) {
        d_ptr->radius = qMin(width(), height()) / 2.0;
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->baseColor);
    painter->drawRoundedRect(rect, d_ptr->radius, d_ptr->radius);

    double width = this->width()
                   * ((d_ptr->value - d_ptr->minValue) / (d_ptr->maxValue - d_ptr->minValue));
    rect.setWidth(width);
    painter->setBrush(d_ptr->chunkColor);
    painter->drawRoundedRect(rect, d_ptr->radius, d_ptr->radius);
}

void ProgressBar::drawText(QPainter *painter)
{
    double min = qMin(width(), height());
    painter->setPen(d_ptr->textColor);
    QFont font("Microsoft YaHei", min / 1.5);
    painter->setFont(font);

    QString strValue;
    if (d_ptr->percent) {
        double temp = d_ptr->value / (d_ptr->maxValue - d_ptr->minValue) * 100;
        strValue = QString("%1%").arg(temp, 0, 'f', 2);
    } else {
        strValue = QString("%1").arg(d_ptr->value, 0, 'f', 2);
    }
    painter->drawText(rect(), Qt::AlignCenter, strValue);
}
