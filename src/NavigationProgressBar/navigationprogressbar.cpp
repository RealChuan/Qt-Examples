#include "navigationprogressbar.h"

#include <QDateTime>
#include <QDebug>
#include <QPainter>
#include <QResizeEvent>

class NavigationProgressBar::NavigationProgressBarPrivate
{
public:
    explicit NavigationProgressBarPrivate(NavigationProgressBar *q)
        : q_ptr(q)
    {
        // 初始化默认步骤信息
        for (int i = 0; i < maxStep; i++)
            topInfo << QString("Step%1").arg(i + 1);

        // 初始化字体
        textFont.setPixelSize(14);
        dateFont.setPixelSize(12);
    }

    NavigationProgressBar *q_ptr;

    QColor backgroundColor = QColor(80, 80, 80);
    QColor foregroundColor = QColor(254, 254, 254);
    QColor currentBackgroundColor = QColor(77, 161, 255);

    QFont textFont;
    QFont dateFont;

    int maxStep = 5;
    int step = 0;
    int spacing = 10;

    QStringList topInfo;
    QStringList dateList;

    QPixmap cachedPixmap;
    bool cacheValid = false;
};

NavigationProgressBar::NavigationProgressBar(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new NavigationProgressBarPrivate(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

NavigationProgressBar::~NavigationProgressBar() = default;

QSize NavigationProgressBar::minimumSizeHint() const
{
    return {300, 100};
}

void NavigationProgressBar::setMessageList(const QStringList &list)
{
    if (list.isEmpty()) {
        qWarning() << "NavigationProgressBar: Message list cannot be empty";
        return;
    }

    d_ptr->topInfo = list;
    d_ptr->maxStep = list.size();

    // 如果当前步骤超出新范围，调整步骤
    if (d_ptr->step > d_ptr->maxStep) {
        d_ptr->step = d_ptr->maxStep;
        d_ptr->dateList = d_ptr->dateList.mid(0, d_ptr->step);
    }

    // 如果日期列表大小不匹配，重建
    if (d_ptr->dateList.size() != d_ptr->maxStep) {
        rebuildDateList();
    }

    invalidateCache();
    update();
}

QStringList NavigationProgressBar::messageList() const
{
    return d_ptr->topInfo;
}

void NavigationProgressBar::setStep(int step)
{
    if (step < 0 || step > d_ptr->maxStep) {
        qWarning() << "NavigationProgressBar: Step out of range:" << step;
        return;
    }

    if (step == d_ptr->step) {
        return; // 没有变化
    }

    if (step > d_ptr->step) {
        // 前进：记录新步骤的时间
        for (int i = d_ptr->step; i < step; i++) {
            if (i < d_ptr->dateList.size()) {
                d_ptr->dateList[i] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            } else {
                d_ptr->dateList.append(
                    QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            }
        }
    } else {
        // 后退：不需要修改日期记录，保持原有日期
        // 如果日期列表大小不匹配，调整
        if (d_ptr->dateList.size() > step) {
            d_ptr->dateList = d_ptr->dateList.mid(0, step);
        }
    }

    // 确保日期列表大小正确
    while (d_ptr->dateList.size() < step) {
        d_ptr->dateList.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }

    d_ptr->step = step;

    invalidateCache();
    update();

    emit stepChanged(step);

    if (step == d_ptr->maxStep) {
        emit progressCompleted();
    }
}

int NavigationProgressBar::step() const
{
    return d_ptr->step;
}

int NavigationProgressBar::maxStep() const
{
    return d_ptr->maxStep;
}

void NavigationProgressBar::setBackgroundColor(const QColor &color)
{
    if (d_ptr->backgroundColor != color) {
        d_ptr->backgroundColor = color;
        invalidateCache();
        update();
    }
}

QColor NavigationProgressBar::backgroundColor() const
{
    return d_ptr->backgroundColor;
}

void NavigationProgressBar::setCurrentBackgroundColor(const QColor &color)
{
    if (d_ptr->currentBackgroundColor != color) {
        d_ptr->currentBackgroundColor = color;
        invalidateCache();
        update();
    }
}

QColor NavigationProgressBar::currentBackgroundColor() const
{
    return d_ptr->currentBackgroundColor;
}

void NavigationProgressBar::setForegroundColor(const QColor &color)
{
    if (d_ptr->foregroundColor != color) {
        d_ptr->foregroundColor = color;
        invalidateCache();
        update();
    }
}

QColor NavigationProgressBar::foregroundColor() const
{
    return d_ptr->foregroundColor;
}

void NavigationProgressBar::setTextFont(const QFont &font)
{
    if (d_ptr->textFont != font) {
        d_ptr->textFont = font;
        invalidateCache();
        update();
    }
}

QFont NavigationProgressBar::textFont() const
{
    return d_ptr->textFont;
}

void NavigationProgressBar::setDateFont(const QFont &font)
{
    if (d_ptr->dateFont != font) {
        d_ptr->dateFont = font;
        invalidateCache();
        update();
    }
}

QFont NavigationProgressBar::dateFont() const
{
    return d_ptr->dateFont;
}

void NavigationProgressBar::setSpacing(int spacing)
{
    if (d_ptr->spacing != spacing && spacing >= 0) {
        d_ptr->spacing = spacing;
        invalidateCache();
        update();
    }
}

int NavigationProgressBar::spacing() const
{
    return d_ptr->spacing;
}

QString NavigationProgressBar::dateAt(int step) const
{
    if (step >= 0 && step < d_ptr->dateList.size()) {
        return d_ptr->dateList.at(step);
    }
    return QString();
}

void NavigationProgressBar::setDateAt(int step, const QString &date)
{
    if (step >= 0 && step < d_ptr->dateList.size()) {
        d_ptr->dateList[step] = date;
        invalidateCache();
        update();
    }
}

void NavigationProgressBar::reset()
{
    d_ptr->step = 0;
    d_ptr->dateList.clear();
    invalidateCache();
    update();
    emit stepChanged(0);
}

void NavigationProgressBar::next()
{
    if (d_ptr->step < d_ptr->maxStep) {
        setStep(d_ptr->step + 1);
    }
}

void NavigationProgressBar::previous()
{
    if (d_ptr->step > 0) {
        setStep(d_ptr->step - 1);
    }
}

void NavigationProgressBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (!d_ptr->cacheValid || d_ptr->cachedPixmap.size() != size()) {
        rebuildCache();
    }

    QPainter painter(this);
    painter.drawPixmap(0, 0, d_ptr->cachedPixmap);
}

void NavigationProgressBar::resizeEvent(QResizeEvent *event)
{
    invalidateCache();
    QWidget::resizeEvent(event);
}

void NavigationProgressBar::drawCompleteProgress(QPainter *painter)
{
    drawBackground(painter, false); // 绘制未完成部分背景
    drawText(painter, false);       // 绘制未完成部分文本
    drawBackground(painter, true);  // 绘制已完成部分背景
    drawText(painter, true);        // 绘制已完成部分文本
}

void NavigationProgressBar::drawBackground(QPainter *painter, const bool completed)
{
    // 圆半径为高度一定比例,计算宽度,将宽度等分
    double totalWidth = width() - d_ptr->spacing * 2;
    double w = totalWidth / d_ptr->maxStep * 1.0;
    double h = height() / 3.0;
    double radius = qMin(w, h) / 2.5; // 稍微减小半径以提供更好的间距
    double initX = d_ptr->spacing;
    double initY = height() / 2.0;

    // 逐个绘制连接线条
    initX += w / 2;

    int step = completed ? d_ptr->step : d_ptr->maxStep;
    int penWidth = radius / (completed ? 6 : 3);
    QColor backgroundColor = completed ? d_ptr->currentBackgroundColor : d_ptr->backgroundColor;

    if (step == 0 && completed) {
        return; // 没有完成的步骤，不需要绘制
    }

    QPen pen(backgroundColor);
    pen.setWidthF(penWidth);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // 绘制连接线
    double currentX = initX;
    for (int i = 0; i < step - 1; i++) {
        painter->drawLine(QPointF(currentX, initY), QPointF(currentX + w, initY));
        currentX += w;
    }

    // 如果当前步骤未完成且不是最后一步，绘制部分连接线
    if (completed && (d_ptr->step > 0) && (d_ptr->step < d_ptr->maxStep)) {
        painter->drawLine(QPointF(currentX, initY), QPointF(currentX + w / 2, initY));
    }

    // 逐个绘制圆
    currentX = initX;
    painter->setPen(Qt::NoPen);
    painter->setBrush(backgroundColor);

    for (int i = 0; i < step; i++) {
        painter->drawEllipse(QPointF(currentX, initY), radius, radius);
        currentX += w;
    }

    // 逐个绘制圆中的数字
    currentX = initX;
    QFont numberFont = d_ptr->textFont;
    numberFont.setPointSizeF(radius * 1.2); // 根据半径调整字体大小
    painter->setFont(numberFont);
    painter->setPen(d_ptr->foregroundColor);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < step; i++) {
        QRectF textRect(currentX - radius, initY - radius, radius * 2, radius * 2);
        painter->drawText(textRect, Qt::AlignCenter, QString::number(i + 1));
        currentX += w;
    }
}

void NavigationProgressBar::drawText(QPainter *painter, const bool completed)
{
    double totalWidth = width() - d_ptr->spacing * 2;
    double w = totalWidth / d_ptr->maxStep * 1.0;
    double h = height() / 3.0;
    double initX = d_ptr->spacing;
    double initY = completed ? h * 2 : 0; // 已完成部分显示在下方，未完成部分显示在上方

    QColor color = completed ? d_ptr->currentBackgroundColor : d_ptr->backgroundColor;
    painter->setFont(completed ? d_ptr->dateFont : d_ptr->textFont);
    painter->setPen(color);
    painter->setBrush(Qt::NoBrush);

    int step = completed ? d_ptr->step : d_ptr->maxStep;

    if (completed && step > d_ptr->dateList.size()) {
        qWarning() << "NavigationProgressBar: Date list size mismatch";
        return;
    }

    for (int i = 0; i < step; i++) {
        QRectF textRect(initX, initY, w, h);
        QString text;

        if (completed) {
            // 显示日期
            text = (i < d_ptr->dateList.size()) ? d_ptr->dateList.at(i) : QString();
        } else {
            // 显示步骤文本
            text = (i < d_ptr->topInfo.size()) ? d_ptr->topInfo.at(i)
                                               : QString("Step %1").arg(i + 1);
        }

        painter->drawText(textRect, Qt::AlignCenter, text);
        initX += w;
    }
}

void NavigationProgressBar::rebuildDateList()
{
    d_ptr->dateList.clear();
    for (int i = 0; i < d_ptr->step; i++) {
        d_ptr->dateList.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
}

void NavigationProgressBar::invalidateCache()
{
    d_ptr->cacheValid = false;
}

void NavigationProgressBar::rebuildCache()
{
    d_ptr->cachedPixmap = QPixmap(size());
    d_ptr->cachedPixmap.fill(Qt::transparent);

    QPainter painter(&d_ptr->cachedPixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    drawCompleteProgress(&painter);

    d_ptr->cacheValid = true;
}
