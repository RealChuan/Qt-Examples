#include "loadingindicator.hpp"

#include <QMovie>
#include <QPainter>
#include <QTimer>
#include <QtMath>

#include <cmath>
#include <numbers>

class LoadingIndicator::LoadingIndicatorPrivate
{
public:
    explicit LoadingIndicatorPrivate(LoadingIndicator *q) : q_ptr(q), timer(new QTimer(q))
    { setupTimer(); }

    ~LoadingIndicatorPrivate() = default;

    void setupTimer()
    {
        timer->setInterval(animationSpeed);
        QObject::connect(timer, &QTimer::timeout, q_ptr, [this]() {
            currentFrame = (currentFrame + 1) % FRAME_MODULUS;

            pulseProgress += PULSE_STEP * pulseDirection;
            if (pulseProgress >= 1.0) {
                pulseProgress = 1.0;
                pulseDirection = -1;
            } else if (pulseProgress <= 0.0) {
                pulseProgress = 0.0;
                pulseDirection = 1;
            }

            q_ptr->update();
        });
    }

    // 计算文本绘制矩形 (位于动画下方，留 TEXT_SPACING 间距)
    [[nodiscard]] auto calculateTextRect(const QRectF &animationRect) const -> QRectF
    {
        if (text.isEmpty()) {
            return {};
        }

        const qreal textTop = animationRect.bottom() + TEXT_SPACING;
        const qreal availableHeight = q_ptr->height() - textTop - TEXT_BOTTOM_MARGIN;
        if (availableHeight <= 0.0) {
            return {};
        }

        const int fontSize
            = qMax(MIN_FONT_PIXEL_SIZE, qMin(q_ptr->width(), q_ptr->height()) / TEXT_FONT_DIVISOR);
        QFont font;
        font.setPixelSize(fontSize);
        font.setBold(true);
        const QFontMetrics fm(font);
        const int textHeight = fm.height();

        const qreal finalTextTop
            = qMin(textTop, static_cast<qreal>(q_ptr->height() - textHeight - TEXT_BOTTOM_MARGIN));

        return QRectF(TEXT_HORIZONTAL_MARGIN,
                      finalTextTop,
                      q_ptr->width() - 2 * TEXT_HORIZONTAL_MARGIN,
                      textHeight);
    }

    LoadingIndicator *q_ptr;

    QTimer *timer;
    std::unique_ptr<QMovie> moviePtr;

    // 动画状态
    int currentFrame = 0;
    double pulseProgress = 0.0;
    int pulseDirection = 1;

    // 配置 (iOS 默认色板)
    AnimationStyle animationStyle = AnimationStyle::RotatingDots;
    QString text;
    QColor textColor = IOS_LABEL;
    QColor color = IOS_SYSTEM_BLUE;
    QColor backgroundColor = IOS_SECONDARY_BG;
    int animationSpeed = ANIMATION_SPEED_DEFAULT;
    int dotCount = DOT_COUNT_DEFAULT;
    int dotRadius = DOT_RADIUS_DEFAULT;
    int barCount = BAR_COUNT_DEFAULT;
    int barWidth = BAR_WIDTH_DEFAULT;

    // === 常量定义 (与 LoadingIndicatorQuick QML 版本同步) ===

    // iOS 风格调色板 (与 CircularProgress 一致)
    static constexpr QColor IOS_SYSTEM_BLUE = QColor(0, 122, 255);         // #007aff
    static constexpr QColor IOS_LABEL = QColor(28, 28, 30);                // #1c1c1e
    static constexpr QColor IOS_SECONDARY_BG = QColor(242, 242, 247, 230); // #F2F2F7E6

    // 几何比例常量 (与 QML 版本完全一致)
    static constexpr double ROTATING_RADIUS_RATIO = 0.25;   // 旋转点半径占 min(w,h) 的比例
    static constexpr double PULSING_MAX_RADIUS_RATIO = 0.3; // 脉冲圆最大半径比例
    static constexpr double PULSING_MIN_SCALE = 0.5;        // 脉冲圆最小缩放
    static constexpr double PULSING_SCALE_RANGE = 0.5;      // 脉冲圆缩放范围 (1.0 - 0.5)
    static constexpr double BAR_MAX_HEIGHT_RATIO = 0.3;     // 弹跳条最大高度比例
    static constexpr double BAR_MIN_SCALE = 0.3;            // 弹跳条最小缩放
    static constexpr double BAR_SCALE_RANGE = 0.7;          // 弹跳条缩放范围 (1.0 - 0.3)

    // 默认参数
    static constexpr int DOT_COUNT_DEFAULT = 8;
    static constexpr int DOT_RADIUS_DEFAULT = 6;
    static constexpr int BAR_COUNT_DEFAULT = 8;
    static constexpr int BAR_WIDTH_DEFAULT = 6;
    static constexpr int ANIMATION_SPEED_DEFAULT = 100;

    // 动画参数
    static constexpr int FRAME_MODULUS = 1000;
    static constexpr double PULSE_STEP = 0.02;
    static constexpr double FRAME_ANGLE_DIVISOR = 60.0; // 旋转点每帧前进角度 = 2π / 60
    static constexpr double BAR_PHASE_SPEED = 0.2;      // 弹跳条相位推进速度
    static constexpr double BAR_PHASE_OFFSET = 0.5;     // 相邻条之间的相位偏移

    // 文本布局
    static constexpr int TEXT_SPACING = 20; // 动画与文本之间的间距
    static constexpr int TEXT_BOTTOM_MARGIN = 10;
    static constexpr int TEXT_HORIZONTAL_MARGIN = 10;
    static constexpr int MIN_FONT_PIXEL_SIZE = 12;
    static constexpr int TEXT_FONT_DIVISOR = 15; // 字体大小 = min(w,h) / 15

    // 输入约束
    static constexpr int MIN_ANIMATION_SPEED = 16; // 约 60 FPS
    static constexpr int MAX_ANIMATION_SPEED = 1000;
    static constexpr int MIN_DOT_COUNT = 3;
    static constexpr int MIN_DOT_RADIUS = 2;
    static constexpr int MIN_BAR_COUNT = 2;
    static constexpr int MIN_BAR_WIDTH = 2;

    // 透明度计算辅助
    static constexpr double OPACITY_BASE = 0.3;
    static constexpr double OPACITY_RANGE = 0.7;
    static constexpr double PULSING_ALPHA_BASE = 0.7;
    static constexpr double PULSING_ALPHA_RANGE = 0.3;
};

LoadingIndicator::LoadingIndicator(QWidget *parent)
    : QWidget(parent), d_ptr(std::make_unique<LoadingIndicatorPrivate>(this))
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(false);
}

LoadingIndicator::LoadingIndicator(AnimationStyle style, QWidget *parent) : LoadingIndicator(parent)
{ setAnimationStyle(style); }

LoadingIndicator::~LoadingIndicator() = default;

void LoadingIndicator::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.fillRect(rect(), d_ptr->backgroundColor);

    QRectF animationRect;
    switch (d_ptr->animationStyle) {
    case AnimationStyle::RotatingDots: animationRect = drawRotatingDots(painter); break;
    case AnimationStyle::PulsingCircle: animationRect = drawPulsingCircle(painter); break;
    case AnimationStyle::BouncingBars: animationRect = drawBouncingBars(painter); break;
    case AnimationStyle::CustomMovie: animationRect = drawMovie(painter); break;
    }

    drawText(painter, animationRect);
}

auto LoadingIndicator::drawRotatingDots(QPainter &painter) -> QRectF
{
    const double minSize = qMin(width(), height());
    const double radius = minSize * d_ptr->ROTATING_RADIUS_RATIO;
    const QPointF center = rect().center();
    const double totalRadius = radius + d_ptr->dotRadius;

    for (int i = 0; i < d_ptr->dotCount; ++i) {
        const double angle
            = 2.0 * std::numbers::pi * i / d_ptr->dotCount
              - 2.0 * std::numbers::pi * d_ptr->currentFrame / d_ptr->FRAME_ANGLE_DIVISOR;
        const double x = center.x() + radius * std::cos(angle);
        const double y = center.y() + radius * std::sin(angle);

        // 透明度随角度脉动，形成"彗尾"效果
        const double opacity
            = d_ptr->OPACITY_BASE
              + d_ptr->OPACITY_RANGE * (std::sin(angle + d_ptr->currentFrame * 0.1) + 1.0) / 2.0;
        QColor dotColor = d_ptr->color;
        dotColor.setAlphaF(opacity);

        painter.setBrush(dotColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(x, y), d_ptr->dotRadius, d_ptr->dotRadius);
    }

    return QRectF(
        center.x() - totalRadius, center.y() - totalRadius, totalRadius * 2.0, totalRadius * 2.0);
}

auto LoadingIndicator::drawPulsingCircle(QPainter &painter) -> QRectF
{
    const QPointF center = rect().center();
    const double maxRadius = qMin(width(), height()) * d_ptr->PULSING_MAX_RADIUS_RATIO;
    const double currentRadius
        = maxRadius
          * (d_ptr->PULSING_MIN_SCALE + d_ptr->PULSING_SCALE_RANGE * d_ptr->pulseProgress);
    const double totalRadius = currentRadius + d_ptr->dotRadius;

    QColor ringColor = d_ptr->color;
    ringColor.setAlphaF(d_ptr->PULSING_ALPHA_BASE
                        + d_ptr->PULSING_ALPHA_RANGE * d_ptr->pulseProgress);

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(ringColor, d_ptr->dotRadius));
    painter.drawEllipse(center, currentRadius, currentRadius);

    return QRectF(
        center.x() - totalRadius, center.y() - totalRadius, totalRadius * 2.0, totalRadius * 2.0);
}

auto LoadingIndicator::drawBouncingBars(QPainter &painter) -> QRectF
{
    const QPointF center = rect().center();
    const double totalWidth = d_ptr->barCount * d_ptr->barWidth * 2.0; // 间距 = barWidth
    const double startX = center.x() - totalWidth / 2.0;
    const double maxHeight = qMin(width(), height()) * d_ptr->BAR_MAX_HEIGHT_RATIO;

    for (int i = 0; i < d_ptr->barCount; ++i) {
        const double progress
            = (std::sin(d_ptr->currentFrame * d_ptr->BAR_PHASE_SPEED + i * d_ptr->BAR_PHASE_OFFSET)
               + 1.0)
              / 2.0;
        const double height
            = maxHeight * (d_ptr->BAR_MIN_SCALE + d_ptr->BAR_SCALE_RANGE * progress);
        const double x = startX + i * d_ptr->barWidth * 2.0;
        const double y = center.y() - height / 2.0;

        QColor barColor = d_ptr->color;
        barColor.setAlphaF(d_ptr->OPACITY_BASE + d_ptr->OPACITY_RANGE * progress);

        painter.setBrush(barColor);
        painter.setPen(Qt::NoPen);
        // iOS 风格圆角条 (胶囊形)
        const double barRadius = d_ptr->barWidth / 2.0;
        painter.drawRoundedRect(QRectF(x, y, d_ptr->barWidth, height), barRadius, barRadius);
    }

    return QRectF(startX, center.y() - maxHeight / 2.0, totalWidth, maxHeight);
}

auto LoadingIndicator::drawMovie(QPainter &painter) -> QRectF
{
    if (!d_ptr->moviePtr) {
        return drawRotatingDots(painter); // 回退到默认动画
    }

    const QPixmap currentPixmap = d_ptr->moviePtr->currentPixmap();
    if (currentPixmap.isNull()) {
        return {};
    }

    const QPoint center = rect().center();
    const int maxWidth = width() / 2;
    const int maxHeight = height() / 2;
    const QSize scaledSize = currentPixmap.size().scaled(maxWidth, maxHeight, Qt::KeepAspectRatio);
    const QRectF movieRect(center.x() - scaledSize.width() / 2,
                           center.y() - scaledSize.height() / 2,
                           scaledSize.width(),
                           scaledSize.height());

    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(movieRect, currentPixmap, currentPixmap.rect());

    return movieRect;
}

void LoadingIndicator::drawText(QPainter &painter, const QRectF &animationRect)
{
    if (d_ptr->text.isEmpty() || animationRect.isEmpty()) {
        return;
    }

    const QRectF textRect = d_ptr->calculateTextRect(animationRect);
    if (textRect.isEmpty()) {
        return;
    }

    painter.setPen(d_ptr->textColor);

    QFont font = painter.font();
    const int fontSize
        = qMax(d_ptr->MIN_FONT_PIXEL_SIZE, qMin(width(), height()) / d_ptr->TEXT_FONT_DIVISOR);
    font.setPixelSize(fontSize);
    font.setBold(true);
    painter.setFont(font);

    const QString displayText = painter.fontMetrics().elidedText(
        d_ptr->text, Qt::ElideRight, static_cast<int>(textRect.width()));
    painter.drawText(textRect, Qt::AlignCenter, displayText);
}

void LoadingIndicator::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (d_ptr->animationStyle == AnimationStyle::CustomMovie) {
        if (d_ptr->moviePtr) {
            d_ptr->moviePtr->start();
        }
        return;
    }

    d_ptr->timer->start(d_ptr->animationSpeed);
}

void LoadingIndicator::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);

    d_ptr->timer->stop();
    if (d_ptr->moviePtr) {
        d_ptr->moviePtr->stop();
    }
}

bool LoadingIndicator::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize && obj == parentWidget()) {
        setGeometry(parentWidget()->rect());
    }
    return QWidget::eventFilter(obj, event);
}

void LoadingIndicator::showOverlay(QWidget *parent)
{
    if (!parent) {
        show();
        return;
    }

    if (parentWidget()) {
        parentWidget()->removeEventFilter(this);
    }

    setParent(parent);
    setGeometry(parent->rect());
    parent->installEventFilter(this);
    show();
    raise();
}

void LoadingIndicator::hideOverlay()
{
    if (parentWidget()) {
        parentWidget()->removeEventFilter(this);
    }
    hide();
}

// === 属性实现 ===

auto LoadingIndicator::text() const -> QString
{ return d_ptr->text; }

void LoadingIndicator::setText(const QString &text)
{
    if (d_ptr->text == text) {
        return;
    }

    d_ptr->text = text;
    setToolTip(text);
    update();
    emit textChanged(text);
}

auto LoadingIndicator::textColor() const -> QColor
{ return d_ptr->textColor; }

void LoadingIndicator::setTextColor(const QColor &color)
{
    if (d_ptr->textColor == color) {
        return;
    }

    d_ptr->textColor = color;
    update();
    emit textColorChanged(color);
}

auto LoadingIndicator::color() const -> QColor
{ return d_ptr->color; }

void LoadingIndicator::setColor(const QColor &color)
{
    if (d_ptr->color == color) {
        return;
    }

    d_ptr->color = color;
    if (d_ptr->animationStyle != AnimationStyle::CustomMovie) {
        update();
    }
    emit colorChanged(color);
}

auto LoadingIndicator::backgroundColor() const -> QColor
{ return d_ptr->backgroundColor; }

void LoadingIndicator::setBackgroundColor(const QColor &color)
{
    if (d_ptr->backgroundColor == color) {
        return;
    }

    d_ptr->backgroundColor = color;
    update();
    emit backgroundColorChanged(color);
}

auto LoadingIndicator::animationSpeed() const -> int
{ return d_ptr->animationSpeed; }

void LoadingIndicator::setAnimationSpeed(int ms)
{
    if (d_ptr->animationSpeed == ms) {
        return;
    }

    d_ptr->animationSpeed = qBound(d_ptr->MIN_ANIMATION_SPEED, ms, d_ptr->MAX_ANIMATION_SPEED);
    d_ptr->timer->setInterval(d_ptr->animationSpeed);
    emit animationSpeedChanged(ms);
}

auto LoadingIndicator::animationStyle() const -> AnimationStyle
{ return d_ptr->animationStyle; }

void LoadingIndicator::setAnimationStyle(AnimationStyle style)
{
    if (d_ptr->animationStyle == style) {
        return;
    }

    if (d_ptr->animationStyle == AnimationStyle::CustomMovie && d_ptr->moviePtr) {
        d_ptr->moviePtr->stop();
    } else {
        d_ptr->timer->stop();
    }

    d_ptr->animationStyle = style;

    if (isVisible()) {
        if (d_ptr->animationStyle == AnimationStyle::CustomMovie && d_ptr->moviePtr) {
            d_ptr->moviePtr->start();
        } else {
            d_ptr->timer->start(d_ptr->animationSpeed);
        }
    }

    update();
    emit animationStyleChanged(style);
}

void LoadingIndicator::setMovie(QMovie *movie)
{
    d_ptr->moviePtr.reset(movie);
    if (!d_ptr->moviePtr) {
        return;
    }

    connect(d_ptr->moviePtr.get(), &QMovie::frameChanged, this, [this](int frameNumber) {
        Q_UNUSED(frameNumber)
        if (d_ptr->animationStyle == AnimationStyle::CustomMovie) {
            update();
        }
    });

    if (isVisible() && d_ptr->animationStyle == AnimationStyle::CustomMovie) {
        d_ptr->moviePtr->start();
    }
}

void LoadingIndicator::setMovie(const QString &fileName)
{
    auto movie = std::make_unique<QMovie>(fileName, QByteArray());
    if (!movie->isValid()) {
        qWarning() << "Failed to load movie:" << fileName;
        return;
    }

    setMovie(movie.release());
    setAnimationStyle(AnimationStyle::CustomMovie);
}

auto LoadingIndicator::dotCount() const -> int
{ return d_ptr->dotCount; }

void LoadingIndicator::setDotCount(int count)
{
    if (d_ptr->dotCount == count) {
        return;
    }

    d_ptr->dotCount = qMax(d_ptr->MIN_DOT_COUNT, count);
    if (d_ptr->animationStyle == AnimationStyle::RotatingDots) {
        update();
    }
}

auto LoadingIndicator::dotRadius() const -> int
{ return d_ptr->dotRadius; }

void LoadingIndicator::setDotRadius(int radius)
{
    if (d_ptr->dotRadius == radius) {
        return;
    }

    d_ptr->dotRadius = qMax(d_ptr->MIN_DOT_RADIUS, radius);
    if (d_ptr->animationStyle == AnimationStyle::RotatingDots
        || d_ptr->animationStyle == AnimationStyle::PulsingCircle) {
        update();
    }
}

auto LoadingIndicator::barCount() const -> int
{ return d_ptr->barCount; }

void LoadingIndicator::setBarCount(int count)
{
    if (d_ptr->barCount == count) {
        return;
    }

    d_ptr->barCount = qMax(d_ptr->MIN_BAR_COUNT, count);
    if (d_ptr->animationStyle == AnimationStyle::BouncingBars) {
        update();
    }
}

auto LoadingIndicator::barWidth() const -> int
{ return d_ptr->barWidth; }

void LoadingIndicator::setBarWidth(int width)
{
    if (d_ptr->barWidth == width) {
        return;
    }

    d_ptr->barWidth = qMax(d_ptr->MIN_BAR_WIDTH, width);
    if (d_ptr->animationStyle == AnimationStyle::BouncingBars) {
        update();
    }
}
