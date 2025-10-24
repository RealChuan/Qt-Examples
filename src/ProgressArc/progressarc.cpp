#include "progressarc.h"

#include <QPainter>
#include <QPropertyAnimation>

struct ProgressArc::ProgressArcPrivate
{
    double minValue = 0;
    double maxValue = 100;
    double value = 0;
    double startAngle = -90;
    double endAngle = 270;

    QColor arcColor = QColor(QLatin1String("#4da1ff"));
    QColor textColor = QColor(QLatin1String("#4da1ff"));
    QColor titleColor = QColor(80, 80, 80);
    QColor baseColor = QColor(179, 179, 179);
    QColor backgroundColor = Qt::transparent;

    bool percent = true;
    QString title;

    QPropertyAnimation *animation;
};

ProgressArc::ProgressArc(const QString &title, QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ProgressArcPrivate)
{
    setTitle(title);
    d_ptr->animation = new QPropertyAnimation(this, "value", this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(this, &ProgressArc::valueChanged, this, &ProgressArc::onStartAnimation);
}

ProgressArc::~ProgressArc() = default;

void ProgressArc::setTitle(const QString &title)
{
    d_ptr->title = title;
    update();
}

auto ProgressArc::title() const -> QString
{
    return d_ptr->title;
}

void ProgressArc::setPercent(const bool percent)
{
    d_ptr->percent = percent;
    update();
}

auto ProgressArc::percent() const -> bool
{
    return d_ptr->percent;
}

void ProgressArc::setMin(const double min)
{
    d_ptr->minValue = min;
    update();
}

auto ProgressArc::min() const -> double
{
    return d_ptr->minValue;
}

void ProgressArc::setmax(const double max)
{
    d_ptr->maxValue = max;
    update();
}

auto ProgressArc::max() const -> double
{
    return d_ptr->maxValue;
}

void ProgressArc::setStartAngle(const double startAngle)
{
    d_ptr->startAngle = startAngle;
    update();
}

auto ProgressArc::startAngle() const -> double
{
    return d_ptr->startAngle;
}

void ProgressArc::setEndAngle(const double endAngle)
{
    d_ptr->endAngle = endAngle;
    update();
}

auto ProgressArc::endAngle() const -> double
{
    return d_ptr->endAngle;
}

void ProgressArc::setArcColor(const QColor &color)
{
    d_ptr->arcColor = color;
    update();
}

auto ProgressArc::arcColor() const -> QColor
{
    return d_ptr->arcColor;
}

void ProgressArc::setTextColor(const QColor &color)
{
    d_ptr->textColor = color;
    update();
}

auto ProgressArc::textColor() const -> QColor
{
    return d_ptr->textColor;
}

void ProgressArc::setTitleColor(const QColor &color)
{
    d_ptr->titleColor = color;
    update();
}

auto ProgressArc::titleColor() const -> QColor
{
    return d_ptr->titleColor;
}

void ProgressArc::setBaseColor(const QColor &color)
{
    d_ptr->baseColor = color;
    update();
}

auto ProgressArc::baseColor() const -> QColor
{
    return d_ptr->baseColor;
}

void ProgressArc::setBackgroundColor(const QColor &color)
{
    d_ptr->backgroundColor = color;
    update();
}

auto ProgressArc::backgroundColor() const -> QColor
{
    return d_ptr->backgroundColor;
}

auto ProgressArc::sizeHint() const -> QSize
{
    return {200, 200};
}

auto ProgressArc::minimumSizeHint() const -> QSize
{
    return {80, 80};
}

void ProgressArc::paintEvent(QPaintEvent *event)
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

    // 圆弧
    drawArc(&painter);

    // text
    drawText(&painter);
}

void ProgressArc::onStartAnimation(const double value)
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

auto ProgressArc::value() const -> double
{
    return d_ptr->value;
}

void ProgressArc::setValue(const double value)
{
    d_ptr->value = value;
    update();
}

void ProgressArc::drawArc(QPainter *painter)
{
    double min = qMin(width(), height());
    double arcWidth = min / 10.0;
    double radius = min / 2.0 - arcWidth - 5;
    QRectF rect = QRectF(-radius, -radius, radius * 2, radius * 2);
    QPen pen;
    pen.setWidthF(arcWidth);
    pen.setCapStyle(Qt::RoundCap);

    // 圆弧背景
    double angle = d_ptr->endAngle - d_ptr->startAngle;
    pen.setColor(d_ptr->baseColor);
    painter->setPen(pen);
    painter->drawArc(rect, d_ptr->startAngle * 16, angle * 16);

    // 圆弧进度
    double spanAngle = angle
                       * ((d_ptr->value - d_ptr->minValue) / (d_ptr->maxValue - d_ptr->minValue));
    double startAngle = d_ptr->endAngle - spanAngle;
    pen.setColor(d_ptr->arcColor);
    painter->setPen(pen);
    painter->drawArc(rect, startAngle * 16, spanAngle * 16);
}

void ProgressArc::drawText(QPainter *painter)
{
    double min = qMin(width(), height());
    double arcWidth = min / 10.0;
    double radius = min / 2.0 - arcWidth - 5;
    painter->setPen(d_ptr->textColor);

    QFont font("Microsoft YaHei", radius / 4.0);
    painter->setFont(font);

    QString strValue;
    if (d_ptr->percent) {
        double temp = d_ptr->value / (d_ptr->maxValue - d_ptr->minValue) * 100;
        strValue = QString("%1%").arg(temp, 0, 'f', 2);
    } else {
        strValue = QString("%1").arg(d_ptr->value, 0, 'f', 2);
    }
    // value
    QRectF valueRect(-radius, 0, radius * 2, radius / 3);
    painter->drawText(valueRect, Qt::AlignCenter, strValue);

    painter->setPen(d_ptr->titleColor);
    font.setPixelSize(radius / 4.0);
    painter->setFont(font);

    // title
    QRectF textRect(-radius, -radius / 2.5, radius * 2, radius / 3);
    painter->drawText(textRect, Qt::AlignCenter, d_ptr->title);
}
