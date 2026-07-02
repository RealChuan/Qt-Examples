#include "navigationprogressbar.hpp"

#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>

#include <cmath>

using namespace Qt::StringLiterals;

class NavigationProgressBar::NavigationProgressBarPrivate
{
public:
    explicit NavigationProgressBarPrivate(NavigationProgressBar *q) : q_ptr(q)
    {
        for (int i = 0; i < maxStep; i++) {
            topInfo << u"Step%1"_s.arg(i + 1);
        }
        textFont.setPixelSize(TEXT_FONT_PIXEL_SIZE);
        dateFont.setPixelSize(DATE_FONT_PIXEL_SIZE);
    }

    NavigationProgressBar *q_ptr;

    // iOS 风格调色板
    static constexpr QColor IOS_SYSTEM_BLUE = QColor(0, 122, 255);       // #007aff
    static constexpr QColor IOS_LABEL = QColor(28, 28, 30);              // #1c1c1e
    static constexpr QColor IOS_SECONDARY_LABEL = QColor(142, 142, 147); // #8e8e93
    static constexpr QColor IOS_SEPARATOR = QColor(209, 209, 214);       // #d1d1d6
    static constexpr QColor IOS_WHITE = QColor(255, 255, 255);

    // 几何比例常量
    static constexpr double HEIGHT_RATIO = 3.0;          // 高度分为 3 部分
    static constexpr double RADIUS_RATIO = 2.5;          // 圆半径 = min(w,h) / 2.5
    static constexpr double COMPLETED_PEN_DIVISOR = 5.0; // 已完成线宽
    static constexpr double FUTURE_PEN_DIVISOR = 8.0;    // 未来线宽
    static constexpr double NUMBER_FONT_SCALE = 0.85;    // 数字字体
    static constexpr double CHECK_FONT_SCALE = 1.1;      // 对勾字体
    static constexpr double RING_OFFSET = 3.0;           // 当前步骤外环偏移

    // 默认字体大小
    static constexpr int TEXT_FONT_PIXEL_SIZE = 14;
    static constexpr int DATE_FONT_PIXEL_SIZE = 11;

    // 默认步骤数与间距
    static constexpr int DEFAULT_MAX_STEP = 5;
    static constexpr int DEFAULT_SPACING = 10;

    // 颜色 (iOS 默认)
    QColor backgroundColor = IOS_SEPARATOR;           // 未来步骤圆/线
    QColor foregroundColor = IOS_WHITE;               // 完成步骤圆内数字/勾
    QColor currentBackgroundColor = IOS_SYSTEM_BLUE;  // 当前步骤 + 已完成
    QColor labelColor = IOS_LABEL;                    // 步骤文字
    QColor secondaryLabelColor = IOS_SECONDARY_LABEL; // 未来步骤文字

    QFont textFont;
    QFont dateFont;

    int maxStep = DEFAULT_MAX_STEP;
    int step = 0;
    int spacing = DEFAULT_SPACING;

    QStringList topInfo;
    QStringList dateList;

    QPixmap cachedPixmap;
    bool cacheValid = false;
};

NavigationProgressBar::NavigationProgressBar(QWidget *parent)
    : QWidget(parent), d_ptr(std::make_unique<NavigationProgressBarPrivate>(this))
{ setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); }

NavigationProgressBar::~NavigationProgressBar() = default;

auto NavigationProgressBar::minimumSizeHint() const -> QSize
{ return {300, 120}; }

auto NavigationProgressBar::setMessageList(const QStringList &list) -> void
{
    if (list.isEmpty()) {
        qWarning() << "NavigationProgressBar: Message list cannot be empty";
        return;
    }

    d_ptr->topInfo = list;
    d_ptr->maxStep = list.size();

    if (d_ptr->step > d_ptr->maxStep) {
        d_ptr->step = d_ptr->maxStep;
        d_ptr->dateList = d_ptr->dateList.mid(0, d_ptr->step);
    }

    if (d_ptr->dateList.size() != d_ptr->maxStep) {
        rebuildDateList();
    }

    invalidateCache();
    update();

    emit messageListChanged();
    emit maxStepChanged(d_ptr->maxStep);
}

auto NavigationProgressBar::messageList() const -> QStringList
{ return d_ptr->topInfo; }

auto NavigationProgressBar::setStep(int step) -> void
{
    if (step < 0 || step > d_ptr->maxStep) {
        qWarning() << "NavigationProgressBar: Step out of range:" << step;
        return;
    }

    if (step == d_ptr->step) {
        return;
    }

    if (step > d_ptr->step) {
        for (int i = d_ptr->step; i < step; i++) {
            if (i < d_ptr->dateList.size()) {
                d_ptr->dateList[i]
                    = QDateTime::currentDateTime().toString(u"yyyy-MM-dd hh:mm:ss"_s);
            } else {
                d_ptr->dateList.append(
                    QDateTime::currentDateTime().toString(u"yyyy-MM-dd hh:mm:ss"_s));
            }
        }
    } else {
        if (d_ptr->dateList.size() > step) {
            d_ptr->dateList = d_ptr->dateList.mid(0, step);
        }
    }

    d_ptr->step = step;

    invalidateCache();
    update();

    emit stepChanged(step);

    if (step == d_ptr->maxStep) {
        emit progressCompleted();
    }
}

auto NavigationProgressBar::step() const -> int
{ return d_ptr->step; }

auto NavigationProgressBar::maxStep() const -> int
{ return d_ptr->maxStep; }

auto NavigationProgressBar::setBackgroundColor(const QColor &color) -> void
{
    if (d_ptr->backgroundColor != color) {
        d_ptr->backgroundColor = color;
        invalidateCache();
        update();
        emit backgroundColorChanged(color);
    }
}

auto NavigationProgressBar::backgroundColor() const -> QColor
{ return d_ptr->backgroundColor; }

auto NavigationProgressBar::setCurrentBackgroundColor(const QColor &color) -> void
{
    if (d_ptr->currentBackgroundColor != color) {
        d_ptr->currentBackgroundColor = color;
        invalidateCache();
        update();
        emit currentBackgroundColorChanged(color);
    }
}

auto NavigationProgressBar::currentBackgroundColor() const -> QColor
{ return d_ptr->currentBackgroundColor; }

auto NavigationProgressBar::setForegroundColor(const QColor &color) -> void
{
    if (d_ptr->foregroundColor != color) {
        d_ptr->foregroundColor = color;
        invalidateCache();
        update();
        emit foregroundColorChanged(color);
    }
}

auto NavigationProgressBar::foregroundColor() const -> QColor
{ return d_ptr->foregroundColor; }

auto NavigationProgressBar::setTextFont(const QFont &font) -> void
{
    if (d_ptr->textFont != font) {
        d_ptr->textFont = font;
        invalidateCache();
        update();
        emit textFontChanged(font);
    }
}

auto NavigationProgressBar::textFont() const -> QFont
{ return d_ptr->textFont; }

auto NavigationProgressBar::setDateFont(const QFont &font) -> void
{
    if (d_ptr->dateFont != font) {
        d_ptr->dateFont = font;
        invalidateCache();
        update();
        emit dateFontChanged(font);
    }
}

auto NavigationProgressBar::dateFont() const -> QFont
{ return d_ptr->dateFont; }

auto NavigationProgressBar::setSpacing(int spacing) -> void
{
    if (d_ptr->spacing != spacing && spacing >= 0) {
        d_ptr->spacing = spacing;
        invalidateCache();
        update();
        emit spacingChanged(spacing);
    }
}

auto NavigationProgressBar::spacing() const -> int
{ return d_ptr->spacing; }

auto NavigationProgressBar::dateAt(int step) const -> QString
{
    if (step >= 0 && step < d_ptr->dateList.size()) {
        return d_ptr->dateList.at(step);
    }
    return {};
}

auto NavigationProgressBar::setDateAt(int step, const QString &date) -> void
{
    if (step >= 0 && step < d_ptr->dateList.size()) {
        d_ptr->dateList[step] = date;
        invalidateCache();
        update();
    }
}

auto NavigationProgressBar::reset() -> void
{
    d_ptr->step = 0;
    d_ptr->dateList.clear();
    invalidateCache();
    update();
    emit stepChanged(0);
}

auto NavigationProgressBar::next() -> void
{
    if (d_ptr->step < d_ptr->maxStep) {
        setStep(d_ptr->step + 1);
    }
}

auto NavigationProgressBar::previous() -> void
{
    if (d_ptr->step > 0) {
        setStep(d_ptr->step - 1);
    }
}

auto NavigationProgressBar::paintEvent(QPaintEvent *event) -> void
{
    Q_UNUSED(event)

    if (!d_ptr->cacheValid || d_ptr->cachedPixmap.size() != size()) {
        rebuildCache();
    }

    QPainter painter(this);
    painter.drawPixmap(0, 0, d_ptr->cachedPixmap);
}

auto NavigationProgressBar::resizeEvent(QResizeEvent *event) -> void
{
    invalidateCache();
    QWidget::resizeEvent(event);
}

auto NavigationProgressBar::drawCompleteProgress(QPainter &painter) -> void
{
    const double totalWidth = width() - d_ptr->spacing * 2;
    const double w = totalWidth / d_ptr->maxStep;
    const double h = height() / d_ptr->HEIGHT_RATIO;
    const double radius = std::min(w, h) / d_ptr->RADIUS_RATIO;
    const double centerY = h + h / 2.0; // 中间行的中心 Y

    // === 1. 绘制连接线 ===
    for (int i = 0; i < d_ptr->maxStep; i++) {
        const double cx = d_ptr->spacing + w * i + w / 2.0;
        const double nextCx = cx + w;

        if (i >= d_ptr->maxStep - 1) {
            break; // 最后一步无后续连接线
        }

        const bool segmentCompleted = (i < d_ptr->step - 1) || (i == d_ptr->step - 1);
        const bool segmentActive = (i == d_ptr->step - 1);

        if (segmentCompleted) {
            // 已完成连接线 - 实线，粗
            QPen pen(d_ptr->currentBackgroundColor);
            pen.setWidthF(radius / d_ptr->COMPLETED_PEN_DIVISOR);
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawLine(QPointF(cx + radius, centerY), QPointF(nextCx - radius, centerY));
        } else {
            // 未来连接线 - 虚线，细
            QPen pen(d_ptr->backgroundColor);
            pen.setWidthF(radius / d_ptr->FUTURE_PEN_DIVISOR);
            pen.setCapStyle(Qt::RoundCap);
            pen.setStyle(Qt::DashLine);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawLine(QPointF(cx + radius, centerY), QPointF(nextCx - radius, centerY));
        }
    }

    // === 2. 绘制圆圈和内容 ===
    for (int i = 0; i < d_ptr->maxStep; i++) {
        const double cx = d_ptr->spacing + w * i + w / 2.0;
        const bool isCompleted = i < d_ptr->step;
        const bool isCurrent = i == d_ptr->step - 1 || (i == 0 && d_ptr->step == 0 && i == 0);

        if (isCompleted) {
            // 已完成步骤 - 实心圆 + 对勾
            painter.setPen(Qt::NoPen);
            painter.setBrush(d_ptr->currentBackgroundColor);
            painter.drawEllipse(QPointF(cx, centerY), radius, radius);

            // 绘制对勾 ✓
            QFont checkFont;
            checkFont.setPixelSize(static_cast<int>(radius * d_ptr->CHECK_FONT_SCALE));
            checkFont.setBold(true);
            painter.setFont(checkFont);
            painter.setPen(d_ptr->foregroundColor);
            painter.setBrush(Qt::NoBrush);

            const QRectF textRect(cx - radius, centerY - radius, radius * 2, radius * 2);
            painter.drawText(textRect, Qt::AlignCenter, u"\u2713"_s); // ✓
        } else {
            // 未来步骤 - 幽灵圆 (描边) + 数字
            QPen ghostPen(d_ptr->backgroundColor);
            ghostPen.setWidthF(radius / d_ptr->FUTURE_PEN_DIVISOR * 1.5);
            painter.setPen(ghostPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(QPointF(cx, centerY), radius, radius);

            // 数字 - 淡色
            QFont numberFont;
            numberFont.setPixelSize(static_cast<int>(radius * d_ptr->NUMBER_FONT_SCALE));
            painter.setFont(numberFont);
            painter.setPen(d_ptr->backgroundColor);
            painter.drawText(QRectF(cx - radius, centerY - radius, radius * 2, radius * 2),
                             Qt::AlignCenter,
                             QString::number(i + 1));
        }
    }

    // === 3. 当前步骤高亮外环 ===
    if (d_ptr->step > 0 && d_ptr->step <= d_ptr->maxStep) {
        const double cx = d_ptr->spacing + w * (d_ptr->step - 1) + w / 2.0;
        const double ringRadius = radius + d_ptr->RING_OFFSET;

        QPen ringPen(d_ptr->currentBackgroundColor);
        ringPen.setWidthF(2.0);
        ringPen.setStyle(Qt::DotLine);
        painter.setPen(ringPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(cx, centerY), ringRadius, ringRadius);
    }

    // === 4. 绘制文本 (上方: 步骤名 / 下方: 日期) ===
    for (int i = 0; i < d_ptr->maxStep; i++) {
        const double cx = d_ptr->spacing + w * i + w / 2.0;
        const bool isCompleted = i < d_ptr->step;

        // 上方: 步骤名
        {
            const QRectF textRect(cx - w / 2.0, 0, w, h);
            QFont stepFont = d_ptr->textFont;
            stepFont.setBold(isCompleted);
            painter.setFont(stepFont);
            painter.setPen(isCompleted ? d_ptr->labelColor : d_ptr->secondaryLabelColor);
            painter.setBrush(Qt::NoBrush);

            const QString stepText
                = (i < d_ptr->topInfo.size()) ? d_ptr->topInfo.at(i) : u"Step %1"_s.arg(i + 1);
            painter.drawText(textRect, Qt::AlignCenter, stepText);
        }

        // 下方: 日期 (仅已完成步骤)
        if (isCompleted && i < d_ptr->dateList.size()) {
            const QRectF dateRect(cx - w / 2.0, h * 2, w, h);
            painter.setFont(d_ptr->dateFont);
            painter.setPen(d_ptr->secondaryLabelColor);
            painter.drawText(dateRect, Qt::AlignCenter, d_ptr->dateList.at(i));
        }
    }
}

auto NavigationProgressBar::rebuildDateList() -> void
{
    d_ptr->dateList.clear();
    for (int i = 0; i < d_ptr->step; i++) {
        d_ptr->dateList.append(QDateTime::currentDateTime().toString(u"yyyy-MM-dd hh:mm:ss"_s));
    }
}

auto NavigationProgressBar::invalidateCache() -> void
{ d_ptr->cacheValid = false; }

auto NavigationProgressBar::rebuildCache() -> void
{
    d_ptr->cachedPixmap = QPixmap(size());
    d_ptr->cachedPixmap.fill(Qt::transparent);

    QPainter painter(&d_ptr->cachedPixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    drawCompleteProgress(painter);

    d_ptr->cacheValid = true;
}
