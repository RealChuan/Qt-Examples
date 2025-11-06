#pragma once

#include <QWidget>

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

public:
    enum class AnimationStyle : int { RotatingDots, PulsingCircle, BouncingBars, CustomMovie };
    Q_ENUM(AnimationStyle)

    explicit LoadingIndicator(QWidget *parent = nullptr);
    explicit LoadingIndicator(AnimationStyle style, QWidget *parent = nullptr);
    ~LoadingIndicator();

    // 主要功能
    void showOverlay(QWidget *parent);
    void hideOverlay();

    // 基本属性
    QString text() const;
    void setText(const QString &text);

    QColor textColor() const;
    void setTextColor(const QColor &color);

    QColor color() const;
    void setColor(const QColor &color);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

    int animationSpeed() const;
    void setAnimationSpeed(int ms);

    AnimationStyle animationStyle() const;
    void setAnimationStyle(AnimationStyle style);

    // 电影动画相关
    void setMovie(const QString &fileName);
    void setMovie(QMovie *movie);

    // 特定动画的参数设置
    void setDotCount(int count);
    void setDotRadius(int radius);
    void setBarCount(int count);
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
    QRect drawRotatingDots(QPainter &painter);
    QRect drawPulsingCircle(QPainter &painter);
    QRect drawBouncingBars(QPainter &painter);
    QRect drawMovie(QPainter &painter);
    void drawText(QPainter &painter, const QRect &animationRect);

    class LoadingIndicatorPrivate;
    QScopedPointer<LoadingIndicatorPrivate> d_ptr;
};
