#include "bubblewidget.hpp"

#include <QApplication>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

using namespace Qt::StringLiterals;

class BubbleWidget::BubbleWidgetPrivate
{
public:
    explicit BubbleWidgetPrivate(BubbleWidget *q) : q_ptr(q)
    {
        // 初始化画笔（细线，暖米 tan）
        pen.setWidth(1);
        pen.setColor(QColor(201, 169, 106)); // #c9a96a

        // 初始化字体（14px 常规）
        textFont.setPixelSize(14);

        // 初始化画刷（manila 暖米，主色）
        brush.setColor(QColor(245, 230, 200)); // #f5e6c8
        brush.setStyle(Qt::SolidPattern);
    }

    ~BubbleWidgetPrivate() = default;

    // 主对象指针
    BubbleWidget *q_ptr;

    // 可配置属性
    int borderRadius = 12;
    int textMargin = 12;

    QPen pen;
    QBrush brush;
    QFont textFont;
    QColor textColor = QColor(74, 55, 40); // #4a3728 deep coffee
    QString text = u"Are you ok!"_s;

    // 三角形属性
    QSize triangleSize = QSize(12, 8);
    Direction direction = Direction::Top;

    // 状态标志
    bool pathDirty = true;

    // 缓存的路径
    QPainterPath bubblePath;

    // 常量定义
    static constexpr int MAX_TEXT_WIDTH_RATIO = 3; // 屏幕宽度的1/3
    static constexpr int MIN_WIDTH = 100;
    static constexpr int MIN_HEIGHT = 50;
    static constexpr int MAX_WIDTH = 500;
    static constexpr int MAX_HEIGHT = 300;
};

BubbleWidget::BubbleWidget(QWidget *parent)
    : QWidget(parent), d_ptr(std::make_unique<BubbleWidgetPrivate>(this))
{ updateSize(); }

BubbleWidget::~BubbleWidget() = default;

// 大小提示
auto BubbleWidget::sizeHint() const -> QSize
{ return calculateOptimalSize(); }

auto BubbleWidget::minimumSizeHint() const -> QSize
{ return QSize(d_ptr->MIN_WIDTH, d_ptr->MIN_HEIGHT); }

// 计算最佳大小
QSize BubbleWidget::calculateOptimalSize() const
{
    // 创建临时字体度量
    QFontMetrics fm(d_ptr->textFont);

    // 获取屏幕可用区域，用于计算最大文本宽度
    QRect screenGeometry = getAvailableScreenGeometry();
    int maxTextWidth = qMin(d_ptr->MAX_WIDTH, screenGeometry.width() / d_ptr->MAX_TEXT_WIDTH_RATIO);

    // 计算文本所需大小（考虑换行）
    QRect textRect = fm.boundingRect(QRect(0, 0, maxTextWidth, d_ptr->MAX_HEIGHT),
                                     Qt::TextWordWrap | Qt::AlignCenter,
                                     d_ptr->text);

    // 计算气泡主体大小（包含边距和阴影）
    int contentWidth = textRect.width() + 2 * (d_ptr->textMargin);
    int contentHeight = textRect.height() + 2 * (d_ptr->textMargin);

    // 确保最小尺寸
    contentWidth = qMax(contentWidth, d_ptr->MIN_WIDTH);
    contentHeight = qMax(contentHeight, d_ptr->MIN_HEIGHT);

    // 确保不超过最大尺寸
    contentWidth = qMin(contentWidth, d_ptr->MAX_WIDTH);
    contentHeight = qMin(contentHeight, d_ptr->MAX_HEIGHT);

    // 根据三角形方向调整总大小
    QSize totalSize;
    switch (d_ptr->direction) {
    case Direction::Top:
    case Direction::Bottom:
        totalSize = QSize(contentWidth, contentHeight + d_ptr->triangleSize.height());
        break;
    case Direction::Left:
    case Direction::Right:
        totalSize = QSize(contentWidth + d_ptr->triangleSize.width(), contentHeight);
        break;
    }

    static constexpr double SHADOW_COMPENSATION_RATIO = 1.2; // 补偿阴影效果
    totalSize.setHeight(totalSize.height() * SHADOW_COMPENSATION_RATIO);
    return totalSize;
}

// 更新大小
void BubbleWidget::updateSize()
{
    QSize newSize = calculateOptimalSize();
    resize(newSize);
    d_ptr->pathDirty = true;
    update();
}

// 画笔设置
void BubbleWidget::setPen(const QPen &pen)
{
    if (d_ptr->pen == pen)
        return;
    d_ptr->pen = pen;
    update();
    emit penChanged(pen);
}

auto BubbleWidget::pen() const -> QPen
{ return d_ptr->pen; }

// 画刷设置
void BubbleWidget::setBrush(const QBrush &brush)
{
    if (d_ptr->brush == brush)
        return;
    d_ptr->brush = brush;
    update();
    emit brushChanged(brush);
}

auto BubbleWidget::brush() const -> QBrush
{ return d_ptr->brush; }

// 边框设置
void BubbleWidget::setBorderRadius(int radius)
{
    if (d_ptr->borderRadius == radius)
        return;
    d_ptr->borderRadius = qMax(0, radius);
    d_ptr->pathDirty = true;
    update();
    emit borderRadiusChanged(radius);
}

auto BubbleWidget::borderRadius() const -> int
{ return d_ptr->borderRadius; }

// 文本设置
void BubbleWidget::setText(const QString &text)
{
    if (d_ptr->text != text) {
        d_ptr->text = text;
        updateSize();
        emit textChanged(text);
    }
}

auto BubbleWidget::text() const -> QString
{ return d_ptr->text; }

void BubbleWidget::setTextMargin(int margin)
{
    if (d_ptr->textMargin == margin)
        return;
    d_ptr->textMargin = qMax(0, margin);
    updateSize();
    emit textMarginChanged(margin);
}

auto BubbleWidget::textMargin() const -> int
{ return d_ptr->textMargin; }

void BubbleWidget::setTextFont(const QFont &font)
{
    if (d_ptr->textFont != font) {
        d_ptr->textFont = font;
        updateSize();
        emit textFontChanged(font);
    }
}

auto BubbleWidget::textFont() const -> QFont
{ return d_ptr->textFont; }

void BubbleWidget::setTextColor(const QColor &color)
{
    if (d_ptr->textColor != color) {
        d_ptr->textColor = color;
        update();
        emit textColorChanged(color);
    }
}

auto BubbleWidget::textColor() const -> QColor
{ return d_ptr->textColor; }

// 三角形设置
void BubbleWidget::setTriangleSize(const QSize &size)
{
    if (d_ptr->triangleSize != size) {
        d_ptr->triangleSize = QSize(qMax(0, size.width()), qMax(0, size.height()));
        d_ptr->pathDirty = true;
        updateSize();
        emit triangleSizeChanged(d_ptr->triangleSize);
    }
}

auto BubbleWidget::triangleSize() const -> QSize
{ return d_ptr->triangleSize; }

void BubbleWidget::setDirection(Direction direction)
{
    if (d_ptr->direction == direction)
        return;
    d_ptr->direction = direction;
    d_ptr->pathDirty = true;
    updateSize();
    emit directionChanged(direction);
}

auto BubbleWidget::direction() const -> Direction
{ return d_ptr->direction; }

// 事件处理
void BubbleWidget::paintEvent([[maybe_unused]] QPaintEvent *event)
{
    if (d_ptr->pathDirty) {
        calculateBubblePath();
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制气泡主体（纯色画刷时使用渐变填充增强纸张/黏土立体感）
    painter.setPen(d_ptr->pen);
    QBrush fillBrush = d_ptr->brush;
    if (fillBrush.style() == Qt::SolidPattern) {
        QRect bubbleRect = calculateBubbleRect();
        QColor baseColor = fillBrush.color();
        QLinearGradient gradient(bubbleRect.topLeft(), bubbleRect.bottomLeft());
        gradient.setColorAt(0, baseColor.lighter(106));
        gradient.setColorAt(1, baseColor.darker(112));
        fillBrush = QBrush(gradient);
    }
    painter.setBrush(fillBrush);
    painter.drawPath(d_ptr->bubblePath);

    // 绘制文本
    painter.setFont(d_ptr->textFont);
    painter.setPen(d_ptr->textColor);

    QRect textRect = calculateTextRect();
    painter.drawText(textRect, Qt::TextWordWrap | Qt::AlignCenter, d_ptr->text);
}

void BubbleWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    d_ptr->pathDirty = true;
}

// 私有方法实现
void BubbleWidget::calculateBubblePath()
{
    d_ptr->bubblePath = QPainterPath();

    // 添加圆角矩形主体
    QRect rect = calculateBubbleRect();
    d_ptr->bubblePath.addRoundedRect(rect, d_ptr->borderRadius, d_ptr->borderRadius);

    // 添加三角形
    QPolygon triangle = calculateTrianglePolygon();
    if (!triangle.isEmpty()) {
        d_ptr->bubblePath.addPolygon(triangle);
    }

    d_ptr->bubblePath = d_ptr->bubblePath.simplified();
    d_ptr->pathDirty = false;
}

auto BubbleWidget::calculateBubbleRect() const -> QRect
{
    const QSize triSize = d_ptr->triangleSize;

    switch (d_ptr->direction) {
    case Direction::Bottom: return QRect(0, triSize.height(), width(), height() - triSize.height());
    case Direction::Top: return QRect(0, 0, width(), height() - triSize.height());
    case Direction::Right: return QRect(triSize.width(), 0, width() - triSize.width(), height());
    case Direction::Left: return QRect(0, 0, width() - triSize.width(), height());
    }

    return QRect(0, 0, width(), height());
}

auto BubbleWidget::calculateTextRect() const -> QRect
{
    QRect bubbleRect = calculateBubbleRect();
    return bubbleRect.adjusted(
        d_ptr->textMargin, d_ptr->textMargin, -d_ptr->textMargin, -d_ptr->textMargin);
}

auto BubbleWidget::calculateTrianglePolygon() const -> QPolygon
{
    QPolygon triangle;
    const QSize triSize = d_ptr->triangleSize;

    if (triSize.width() <= 0 || triSize.height() <= 0) {
        return triangle;
    }

    switch (d_ptr->direction) {
    case Direction::Bottom:
        triangle << QPoint(width() / 2, 0)
                 << QPoint(width() / 2 + triSize.width() / 2, triSize.height())
                 << QPoint(width() / 2 - triSize.width() / 2, triSize.height());
        break;
    case Direction::Top:
        triangle << QPoint(width() / 2, height())
                 << QPoint(width() / 2 + triSize.width() / 2, height() - triSize.height())
                 << QPoint(width() / 2 - triSize.width() / 2, height() - triSize.height());
        break;
    case Direction::Right:
        triangle << QPoint(0, height() / 2)
                 << QPoint(triSize.width(), height() / 2 - triSize.height() / 2)
                 << QPoint(triSize.width(), height() / 2 + triSize.height() / 2);
        break;
    case Direction::Left:
        triangle << QPoint(width(), height() / 2)
                 << QPoint(width() - triSize.width(), height() / 2 - triSize.height() / 2)
                 << QPoint(width() - triSize.width(), height() / 2 + triSize.height() / 2);
        break;
    }

    return triangle;
}

auto BubbleWidget::getAvailableScreenGeometry() const -> QRect
{
    QWidget *parent = parentWidget();
    if (parent) {
        QScreen *screen = parent->screen();
        if (screen) {
            return screen->availableGeometry();
        }
    }

    // 回退到桌面可用区域
    return QApplication::primaryScreen()->availableGeometry();
}
