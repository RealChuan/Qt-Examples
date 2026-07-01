#pragma once

#include <QWidget>

#include <memory>

using namespace Qt::StringLiterals;

class QMovie;

class LoadingIndicator : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY
                   backgroundColorChanged)
    Q_PROPERTY(
        int animationSpeed READ animationSpeed WRITE setAnimationSpeed NOTIFY animationSpeedChanged)
    Q_PROPERTY(AnimationStyle animationStyle READ animationStyle WRITE setAnimationStyle NOTIFY
                   animationStyleChanged)

public:
    enum class AnimationStyle : int { RotatingDots, PulsingCircle, BouncingBars, CustomMovie };
    Q_ENUM(AnimationStyle)

    explicit LoadingIndicator(QWidget *parent = nullptr);
    explicit LoadingIndicator(AnimationStyle style, QWidget *parent = nullptr);
    ~LoadingIndicator() override;
    LoadingIndicator(const LoadingIndicator &) = delete;
    auto operator=(const LoadingIndicator &) -> LoadingIndicator & = delete;
    LoadingIndicator(LoadingIndicator &&) = delete;
    auto operator=(LoadingIndicator &&) -> LoadingIndicator & = delete;

    // 覆盖层模式
    void showOverlay(QWidget *parent);
    void hideOverlay();

    // 文本
    [[nodiscard]] auto text() const -> QString;
    void setText(const QString &text);

    // 颜色
    [[nodiscard]] auto textColor() const -> QColor;
    void setTextColor(const QColor &color);

    [[nodiscard]] auto color() const -> QColor;
    void setColor(const QColor &color);

    [[nodiscard]] auto backgroundColor() const -> QColor;
    void setBackgroundColor(const QColor &color);

    // 动画
    [[nodiscard]] auto animationSpeed() const -> int;
    void setAnimationSpeed(int ms);

    [[nodiscard]] auto animationStyle() const -> AnimationStyle;
    void setAnimationStyle(AnimationStyle style);

    // 自定义 GIF 动画 (所有权转移到本控件)
    void setMovie(QMovie *movie);
    void setMovie(const QString &fileName);

    // 动画几何参数
    [[nodiscard]] auto dotCount() const -> int;
    void setDotCount(int count);

    [[nodiscard]] auto dotRadius() const -> int;
    void setDotRadius(int radius);

    [[nodiscard]] auto barCount() const -> int;
    void setBarCount(int count);

    [[nodiscard]] auto barWidth() const -> int;
    void setBarWidth(int width);

signals:
    void textChanged(const QString &text);
    void textColorChanged(const QColor &color);
    void colorChanged(const QColor &color);
    void backgroundColorChanged(const QColor &color);
    void animationSpeedChanged(int ms);
    void animationStyleChanged(AnimationStyle style);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    auto drawRotatingDots(QPainter &painter) -> QRectF;
    auto drawPulsingCircle(QPainter &painter) -> QRectF;
    auto drawBouncingBars(QPainter &painter) -> QRectF;
    auto drawMovie(QPainter &painter) -> QRectF;
    void drawText(QPainter &painter, const QRectF &animationRect);

    class LoadingIndicatorPrivate;
    std::unique_ptr<LoadingIndicatorPrivate> d_ptr;
};
