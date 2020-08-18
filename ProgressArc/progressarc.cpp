#include "progressarc.h"

#include <QPainter>
#include <QPropertyAnimation>

struct ProgressArcPrivate{
    double minValue = 0;
    double maxValue = 100;
    double value = 0;
    double startAngle = -90;
    double endAngle = 270;

    QColor arcColor = QColor("#4da1ff");
    QColor textColor = QColor("#4da1ff");
    QColor titleColor = QColor(80, 80, 80);
    QColor baseColor = QColor(179,179,179);
    QColor backgroundColor = Qt::transparent;

    bool percent = true;
    QString title;

    QPropertyAnimation *animation;
};

ProgressArc::ProgressArc(const QString &title, QWidget *parent)
    : QWidget(parent)
    , d(new ProgressArcPrivate)
{
    setTitle(title);
    d->animation = new QPropertyAnimation(this, "value", this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(this, &ProgressArc::valueChanged, this, &ProgressArc::onStartAnimation);
}

ProgressArc::~ProgressArc()
{
}

void ProgressArc::setTitle(const QString &title)
{
    d->title = title;
    update();
}

QString ProgressArc::title() const
{
    return d->title;
}

void ProgressArc::setPercent(const bool percent)
{
    d->percent = percent;
    update();
}

bool ProgressArc::percent() const
{
    return d->percent;
}

void ProgressArc::setMin(const double min)
{
    d->minValue = min;
    update();
}

double ProgressArc::min() const
{
    return d->minValue;
}

void ProgressArc::setmax(const double max)
{
    d->maxValue = max;
    update();
}

double ProgressArc::max() const
{
    return d->maxValue;
}

void ProgressArc::setStartAngle(const double startAngle)
{
    d->startAngle = startAngle;
    update();
}

double ProgressArc::startAngle() const
{
    return d->startAngle;
}

void ProgressArc::setEndAngle(const double endAngle)
{
    d->endAngle = endAngle;
    update();
}

double ProgressArc::endAngle() const
{
    return d->endAngle;
}

void ProgressArc::setArcColor(const QColor &color)
{
    d->arcColor = color;
    update();
}

QColor ProgressArc::arcColor() const
{
    return d->arcColor;
}

void ProgressArc::setTextColor(const QColor &color)
{
    d->textColor = color;
    update();
}

QColor ProgressArc::textColor() const
{
    return d->textColor;
}

void ProgressArc::setTitleColor(const QColor &color)
{
    d->titleColor = color;
    update();
}

QColor ProgressArc::titleColor() const
{
    return d->titleColor;
}

void ProgressArc::setBaseColor(const QColor &color)
{
    d->baseColor = color;
    update();
}

QColor ProgressArc::baseColor() const
{
    return d->baseColor;
}

void ProgressArc::setBackgroundColor(const QColor &color)
{
    d->backgroundColor = color;
    update();
}

QColor ProgressArc::backgroundColor() const
{
    return d->backgroundColor;
}

QSize ProgressArc::sizeHint() const
{
    return QSize(200, 200);
}

QSize ProgressArc::minimumSizeHint() const
{
    return QSize(80, 80);
}

void ProgressArc::paintEvent(QPaintEvent *event)
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

    // 圆弧
    drawArc(&painter);

    // text
    drawText(&painter);
}

void ProgressArc::onStartAnimation(const double value)
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

double ProgressArc::value() const
{
    return d->value;
}

void ProgressArc::setValue(const double value)
{
    d->value = value;
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
    double angle = d->endAngle - d->startAngle;
    pen.setColor(d->baseColor);
    painter->setPen(pen);
    painter->drawArc(rect, d->startAngle * 16, angle * 16);

    // 圆弧进度
    double spanAngle = angle * ((d->value - d->minValue) / (d->maxValue - d->minValue));
    double startAngle = d->endAngle - spanAngle;
    pen.setColor(d->arcColor);
    painter->setPen(pen);
    painter->drawArc(rect, startAngle * 16, spanAngle * 16);
}

void ProgressArc::drawText(QPainter *painter)
{
    double min = qMin(width(), height());
    double arcWidth = min / 10.0;
    double radius = min / 2.0 - arcWidth - 5;
    painter->setPen(d->textColor);

    QFont font("Microsoft YaHei", radius / 4.0);
    painter->setFont(font);

    QString strValue;
    if (d->percent) {
        double temp = d->value / (d->maxValue - d->minValue) * 100;
        strValue = QString("%1%").arg(temp, 0, 'f', 2);
    } else
        strValue = QString("%1").arg(d->value, 0, 'f', 2);

    // value
    QRectF valueRect(-radius, 0, radius * 2, radius / 3);
    painter->drawText(valueRect, Qt::AlignCenter, strValue);

    painter->setPen(d->titleColor);
    font.setPixelSize(radius / 4.0);
    painter->setFont(font);

    // title
    QRectF textRect(-radius, -radius / 2.5, radius * 2, radius / 3);
    painter->drawText(textRect, Qt::AlignCenter, d->title);
}
