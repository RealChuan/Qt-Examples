#pragma once

#include <QPen>
#include <QWidget>

#include <memory>

class BubbleWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPen pen READ pen WRITE setPen NOTIFY penChanged)
    Q_PROPERTY(QBrush brush READ brush WRITE setBrush NOTIFY brushChanged)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(
        QSize triangleSize READ triangleSize WRITE setTriangleSize NOTIFY triangleSizeChanged)
    Q_PROPERTY(Direction direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(int textMargin READ textMargin WRITE setTextMargin NOTIFY textMarginChanged)
    Q_PROPERTY(QFont textFont READ textFont WRITE setTextFont NOTIFY textFontChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)

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

signals:
    void penChanged(const QPen &pen);
    void brushChanged(const QBrush &brush);
    void borderRadiusChanged(int radius);
    void textChanged(const QString &text);
    void triangleSizeChanged(const QSize &size);
    void directionChanged(Direction direction);
    void textMarginChanged(int margin);
    void textFontChanged(const QFont &font);
    void textColorChanged(const QColor &color);

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
    std::unique_ptr<BubbleWidgetPrivate> d_ptr;
};
