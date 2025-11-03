#pragma once

#include <QPen>
#include <QWidget>

class BubbleWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPen pen READ pen WRITE setPen)
    Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QSize triangleSize READ triangleSize WRITE setTriangleSize)
    Q_PROPERTY(Direction direction READ direction WRITE setDirection)
    Q_PROPERTY(int textMargin READ textMargin WRITE setTextMargin)
    Q_PROPERTY(QFont textFont READ textFont WRITE setTextFont)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)

public:
    enum class Direction : int { Left, Right, Top, Bottom };
    Q_ENUM(Direction)

    explicit BubbleWidget(QWidget *parent = nullptr);
    ~BubbleWidget() override;

    // 大小提示
    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;
    [[nodiscard]] auto calculateOptimalSize() const -> QSize;
    void updateSize();

    // 画笔设置
    void setPen(const QPen &pen);
    [[nodiscard]] auto pen() const -> QPen;

    // 画刷设置
    void setBrush(const QBrush &brush);
    [[nodiscard]] auto brush() const -> QBrush;

    // 边框设置
    void setBorderRadius(int radius);
    [[nodiscard]] auto borderRadius() const -> int;

    // 文本设置
    void setText(const QString &text);
    [[nodiscard]] auto text() const -> QString;

    void setTextMargin(int margin);
    [[nodiscard]] auto textMargin() const -> int;

    void setTextFont(const QFont &font);
    [[nodiscard]] auto textFont() const -> QFont;

    void setTextColor(const QColor &color);
    [[nodiscard]] auto textColor() const -> QColor;

    // 三角形设置
    void setTriangleSize(const QSize &size);
    [[nodiscard]] auto triangleSize() const -> QSize;

    void setDirection(Direction direction);
    [[nodiscard]] auto direction() const -> Direction;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void calculateBubblePath();
    [[nodiscard]] auto calculateBubbleRect() const -> QRect;
    [[nodiscard]] auto calculateTextRect() const -> QRect;
    [[nodiscard]] auto calculateTrianglePolygon() const -> QPolygon;
    [[nodiscard]] auto getAvailableScreenGeometry() const -> QRect;

    class BubbleWidgetPrivate;
    QScopedPointer<BubbleWidgetPrivate> d_ptr;
};
