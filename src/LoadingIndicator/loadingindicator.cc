#include "loadingindicator.hpp"

#include <QMovie>
#include <QPainter>
#include <QTimer>
#include <QtMath>

class LoadingIndicator::LoadingIndicatorPrivate
{
public:
    explicit LoadingIndicatorPrivate(LoadingIndicator *q)
        : q_ptr(q)
        , timer(new QTimer(q))
    {
        setupTimer();
    }

    ~LoadingIndicatorPrivate() = default;

    void setupTimer()
    {
        timer->setInterval(animationSpeed);
        q_ptr->connect(timer, &QTimer::timeout, q_ptr, [this]() {
            currentFrame = (currentFrame + 1) % 1000;

            // 更新脉冲动画进度
            pulseProgress += 0.02 * pulseDirection;
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

    QRect calculateTextRect(const QRect &animationRect) const
    {
        if (text.isEmpty()) {
            return QRect();
        }

        // 文本显示在动画下方，至少留出20像素间距
        const int textTop = animationRect.bottom() + 20;
        const int availableHeight = q_ptr->height() - textTop - 10; // 底部留10像素边距

        if (availableHeight <= 0) {
            return QRect(); // 没有足够空间显示文本
        }

        // 计算合适的字体大小
        const int fontSize = qMax(12, qMin(q_ptr->width(), availableHeight) / 20);
        QFont font;
        font.setPixelSize(fontSize);
        QFontMetrics fm(font);
        const int textHeight = fm.height();

        // 确保文本不会超出底部
        const int finalTextTop = qMin(textTop, q_ptr->height() - textHeight - 10);

        return QRect(10, finalTextTop, q_ptr->width() - 20, textHeight);
    }

    LoadingIndicator *q_ptr;

    QTimer *timer;
    QScopedPointer<QMovie> moviePtr;

    // 动画状态
    int currentFrame = 0;
    double pulseProgress = 0.0;
    int pulseDirection = 1;

    // 配置
    AnimationStyle animationStyle = AnimationStyle::RotatingDots;
    QString text;
    QColor textColor = QColor(4, 181, 200);
    QColor color = QColor(70, 130, 230);
    QColor backgroundColor = QColor(255, 255, 255, 100);
    int animationSpeed = 100;
    int dotCount = 8;
    int dotRadius = 6;
    int barCount = 8;
    int barWidth = 6;
};

LoadingIndicator::LoadingIndicator(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new LoadingIndicatorPrivate(this))
{}

LoadingIndicator::LoadingIndicator(AnimationStyle style, QWidget *parent)
    : LoadingIndicator(parent)
{
    setAnimationStyle(style);
}

LoadingIndicator::~LoadingIndicator() = default;

void LoadingIndicator::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    painter.fillRect(rect(), d_ptr->backgroundColor);

    // 根据样式绘制动画并获取动画区域
    QRect animationRect;
    switch (d_ptr->animationStyle) {
    case AnimationStyle::RotatingDots: animationRect = drawRotatingDots(painter); break;
    case AnimationStyle::PulsingCircle: animationRect = drawPulsingCircle(painter); break;
    case AnimationStyle::BouncingBars: animationRect = drawBouncingBars(painter); break;
    case AnimationStyle::CustomMovie: animationRect = drawMovie(painter); break;
    }

    // 绘制文本
    drawText(painter, animationRect);
}

QRect LoadingIndicator::drawRotatingDots(QPainter &painter)
{
    const double radius = qMin(width(), height()) * 0.25;
    const QPointF center = rect().center();
    const double totalRadius = radius + d_ptr->dotRadius;

    for (int i = 0; i < d_ptr->dotCount; ++i) {
        const double angle = 2 * M_PI * i / d_ptr->dotCount - 2 * M_PI * d_ptr->currentFrame / 60.0;
        const double x = center.x() + radius * qCos(angle);
        const double y = center.y() + radius * qSin(angle);

        const double opacity = 0.3 + 0.7 * (qSin(angle + d_ptr->currentFrame * 0.1) + 1) / 2;
        QColor color = d_ptr->color;
        color.setAlphaF(opacity);

        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(x, y), d_ptr->dotRadius, d_ptr->dotRadius);
    }

    return QRect(center.x() - totalRadius,
                 center.y() - totalRadius,
                 totalRadius * 2,
                 totalRadius * 2);
}

QRect LoadingIndicator::drawPulsingCircle(QPainter &painter)
{
    const QPointF center = rect().center();
    const double maxRadius = qMin(width(), height()) * 0.3;
    const double currentRadius = maxRadius * (0.5 + 0.5 * d_ptr->pulseProgress);
    const double totalRadius = currentRadius + d_ptr->dotRadius; // 考虑线宽

    QColor color = d_ptr->color;
    color.setAlphaF(0.7 + 0.3 * d_ptr->pulseProgress);

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(color, d_ptr->dotRadius));
    painter.drawEllipse(center, currentRadius, currentRadius);

    return QRect(center.x() - totalRadius,
                 center.y() - totalRadius,
                 totalRadius * 2,
                 totalRadius * 2);
}

QRect LoadingIndicator::drawBouncingBars(QPainter &painter)
{
    const QPointF center = rect().center();
    const double totalWidth = d_ptr->barCount * d_ptr->barWidth * 2;
    const double startX = center.x() - totalWidth / 2;
    const double maxHeight = qMin(width(), height()) * 0.3;

    for (int i = 0; i < d_ptr->barCount; ++i) {
        const double progress = (qSin((d_ptr->currentFrame * 0.2) + i * 0.5) + 1) / 2;
        const double height = maxHeight * (0.3 + 0.7 * progress);
        const double x = startX + i * d_ptr->barWidth * 2;
        const double y = center.y() - height / 2;

        QColor color = d_ptr->color;
        color.setAlphaF(0.3 + 0.7 * progress);

        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        painter.drawRect(QRectF(x, y, d_ptr->barWidth, height));
    }

    return QRect(startX, center.y() - maxHeight / 2, totalWidth, maxHeight);
}

QRect LoadingIndicator::drawMovie(QPainter &painter)
{
    if (!d_ptr->moviePtr) {
        return drawRotatingDots(painter); // 回退到默认动画
    }

    const QPixmap currentFrame = d_ptr->moviePtr->currentPixmap();
    if (currentFrame.isNull()) {
        return QRect();
    }

    const QPoint center = rect().center();
    const int maxWidth = width() / 2;
    const int maxHeight = height() / 2;
    const QSize scaledSize = currentFrame.size().scaled(maxWidth, maxHeight, Qt::KeepAspectRatio);
    const QRect movieRect(center.x() - scaledSize.width() / 2,
                          center.y() - scaledSize.height() / 2,
                          scaledSize.width(),
                          scaledSize.height());

    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(movieRect, currentFrame);

    return movieRect;
}

void LoadingIndicator::drawText(QPainter &painter, const QRect &animationRect)
{
    if (d_ptr->text.isEmpty() || animationRect.isEmpty()) {
        return;
    }

    const QRect textRect = d_ptr->calculateTextRect(animationRect);
    if (textRect.isEmpty()) {
        return;
    }

    painter.setPen(d_ptr->textColor);

    // 设置合适的字体大小
    QFont font = painter.font();
    const int fontSize = qMax(12, qMin(width(), textRect.height()) / 12);
    font.setPixelSize(fontSize);
    font.setBold(true);
    painter.setFont(font);

    const QString displayText = painter.fontMetrics().elidedText(d_ptr->text,
                                                                 Qt::ElideRight,
                                                                 textRect.width());
    painter.setRenderHint(QPainter::TextAntialiasing);
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

// 属性设置实现
QString LoadingIndicator::text() const
{
    return d_ptr->text;
}

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

QColor LoadingIndicator::textColor() const
{
    return d_ptr->textColor;
}

void LoadingIndicator::setTextColor(const QColor &color)
{
    if (d_ptr->textColor == color) {
        return;
    }

    d_ptr->textColor = color;
    update();
    emit textColorChanged(color);
}

QColor LoadingIndicator::color() const
{
    return d_ptr->color;
}

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

QColor LoadingIndicator::backgroundColor() const
{
    return d_ptr->backgroundColor;
}

void LoadingIndicator::setBackgroundColor(const QColor &color)
{
    if (d_ptr->backgroundColor == color) {
        return;
    }

    d_ptr->backgroundColor = color;
    update();
    emit backgroundColorChanged(color);
}

int LoadingIndicator::animationSpeed() const
{
    return d_ptr->animationSpeed;
}

void LoadingIndicator::setAnimationSpeed(int ms)
{
    if (d_ptr->animationSpeed == ms) {
        return;
    }

    d_ptr->animationSpeed = qBound(16, ms, 1000);
    d_ptr->timer->setInterval(d_ptr->animationSpeed);
    emit animationSpeedChanged(ms);
}

LoadingIndicator::AnimationStyle LoadingIndicator::animationStyle() const
{
    return d_ptr->animationStyle;
}

void LoadingIndicator::setAnimationStyle(AnimationStyle style)
{
    if (d_ptr->animationStyle == style) {
        return;
    }

    // 停止当前动画
    if (d_ptr->animationStyle == AnimationStyle::CustomMovie && d_ptr->moviePtr) {
        d_ptr->moviePtr->stop();
    } else {
        d_ptr->timer->stop();
    }

    d_ptr->animationStyle = style;

    // 启动新动画
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

    connect(d_ptr->moviePtr.data(), &QMovie::frameChanged, this, [this](int frameNumber) {
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
    std::unique_ptr<QMovie> movie(new QMovie(fileName, QByteArray()));
    if (!movie->isValid()) {
        qWarning() << "Failed to load movie:" << fileName;
        return;
    }

    setMovie(movie.release());
    setAnimationStyle(AnimationStyle::CustomMovie);
}

void LoadingIndicator::setDotCount(int count)
{
    if (d_ptr->dotCount == count) {
        return;
    }

    d_ptr->dotCount = qMax(3, count);
    if (d_ptr->animationStyle == AnimationStyle::RotatingDots) {
        update();
    }
}

void LoadingIndicator::setDotRadius(int radius)
{
    if (d_ptr->dotRadius == radius) {
        return;
    }

    d_ptr->dotRadius = qMax(2, radius);
    if (d_ptr->animationStyle == AnimationStyle::RotatingDots) {
        update();
    }
}

void LoadingIndicator::setBarCount(int count)
{
    if (d_ptr->barCount == count) {
        return;
    }

    d_ptr->barCount = qMax(2, count);
    if (d_ptr->animationStyle == AnimationStyle::BouncingBars) {
        update();
    }
}

void LoadingIndicator::setBarWidth(int width)
{
    if (d_ptr->barWidth == width) {
        return;
    }

    d_ptr->barWidth = qMax(2, width);
    if (d_ptr->animationStyle == AnimationStyle::BouncingBars) {
        update();
    }
}
