#include "navigationprogressbar.h"

#include <QDateTime>
#include <QDebug>
#include <QPainter>

class NavigationProgressBarPrivate
{
public:
    explicit NavigationProgressBarPrivate(QWidget *parent)
        : owner(parent)
    {
        for (int i = 0; i < maxStep; i++)
            topInfo << QString("Step%1").arg(i + 1);
    }
    QWidget *owner;
    QColor backgroundColor = QColor(80, 80, 80);
    QColor foregroundColor = QColor(254, 254, 254);
    QColor currentBackgroundColor = QColor(QLatin1String("#4da1ff"));
    int maxStep = 5;
    int step = 0;
    QStringList topInfo;
    QStringList dateList;
};

NavigationProgressBar::NavigationProgressBar(QWidget *parent)
    : QWidget(parent)
    , d(new NavigationProgressBarPrivate(this))
{}

NavigationProgressBar::~NavigationProgressBar() = default;

auto NavigationProgressBar::sizeHint() const -> QSize
{
    return {500, 100};
}

auto NavigationProgressBar::minimumSizeHint() const -> QSize
{
    return {300, 75};
}

void NavigationProgressBar::setMessageList(const QStringList &list)
{
    d->topInfo = list;
    d->maxStep = list.size();
    update();
}

auto NavigationProgressBar::messageList() const -> QStringList
{
    return d->topInfo;
}

void NavigationProgressBar::setStep(const int step)
{
    if (d->step >= step || step > d->maxStep) {
        return;
}

    int s = step - d->step;
    for (int i = 0; i < s; i++)
        d->dateList.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    d->step = step;
    Q_ASSERT(d->step == d->dateList.size());
    update();
}

auto NavigationProgressBar::step() const -> int
{
    return d->step;
}

void NavigationProgressBar::setBackgroundColor(const QColor &color)
{
    d->backgroundColor = color;
    update();
}

auto NavigationProgressBar::backgroundColor() const -> QColor
{
    return d->backgroundColor;
}

void NavigationProgressBar::setCurrentBackgroundColor(const QColor &color)
{
    d->currentBackgroundColor = color;
    update();
}

auto NavigationProgressBar::currentBackgroundColor() const -> QColor
{
    return d->currentBackgroundColor;
}

void NavigationProgressBar::setForegroundColor(const QColor &color)
{
    d->foregroundColor = color;
    update();
}

auto NavigationProgressBar::foregroundColor() const -> QColor
{
    return d->foregroundColor;
}

void NavigationProgressBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    drawBackground(&painter, false);
    drawText(&painter, false);
    drawBackground(&painter, true);
    drawText(&painter, true);
}

void NavigationProgressBar::drawBackground(QPainter *painter, const bool ok)
{
    //圆半径为高度一定比例,计算宽度,将宽度等分
    double w = width() / d->maxStep * 1.0;
    double h = height() / 3 * 1.0;
    double radius = qMin(w, h) / 2;
    double initX = 0;
    double initY = height() / 2.0;

    //逐个绘制连接线条
    initX = w / 2;

    int step = d->maxStep;
    int penWidth = radius / 4;
    QColor backgroundColor = d->backgroundColor;
    if (ok) {
        step = d->step;
        penWidth = radius / 8;
        backgroundColor = d->currentBackgroundColor;
        radius = radius / 7 * 6;
    }

    QPen pen(backgroundColor);
    pen.setWidthF(penWidth);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < step - 1; i++) {
        painter->drawLine(QPoint(initX, initY), QPoint(initX + w, initY));
        initX += w;
    }

    if (ok && (d->step > 0) && (d->step < d->maxStep)) {
        painter->drawLine(QPoint(initX, initY), QPoint(initX + w / 2, initY));
}

    //逐个绘制圆
    initX = w / 2;
    painter->setPen(Qt::NoPen);
    painter->setBrush(backgroundColor);

    for (int i = 0; i < step; i++) {
        painter->drawEllipse(QPointF(initX, initY), radius, radius);
        initX += w;
    }

    //逐个绘制圆中的数字
    initX = w / 2;
    QFont font("Microsoft YaHei", radius);
    painter->setFont(font);
    painter->setPen(d->foregroundColor);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < step; i++) {
        QRect textRect(initX - radius, initY - radius, radius * 2, radius * 2);
        painter->drawText(textRect, Qt::AlignCenter, QString::number(i + 1));
        initX += w;
    }
}

void NavigationProgressBar::drawText(QPainter *painter, const bool ok)
{
    double w = width() / d->maxStep * 1.0;
    double h = height() / 3.0;
    double initX = 0;
    double initY = 0;

    QColor color = ok ? d->currentBackgroundColor : d->backgroundColor;
    painter->setFont(QFont("Microsoft YaHei", h / 4));
    painter->setPen(color);
    painter->setBrush(Qt::NoBrush);

    int step = ok ? d->step : d->maxStep;
    for (int i = 0; i < step; i++) {
        QRect textRect(initX, initY, w, h);
        painter->drawText(textRect, Qt::AlignCenter, d->topInfo.at(i));
        initX += w;
    }

    if (ok) {
        initX = 0;
        initY = h * 2;
        for (int i = 0; i < step; i++) {
            QRect textRect(initX, initY, w, h);
            painter->drawText(textRect, Qt::AlignCenter, d->dateList.at(i));
            initX += w;
        }
    }
}
