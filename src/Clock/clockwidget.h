#pragma once

#include <QWidget>

class ClockWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY
                   backgroundColorChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY
                   foregroundColorChanged)
    Q_PROPERTY(QColor hourColor READ hourColor WRITE setHourColor NOTIFY hourColorChanged)
    Q_PROPERTY(QColor minuteColor READ minuteColor WRITE setMinuteColor NOTIFY minuteColorChanged)
    Q_PROPERTY(QColor secondColor READ secondColor WRITE setSecondColor NOTIFY secondColorChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(bool smoothAnimation READ smoothAnimation WRITE setSmoothAnimation NOTIFY
                   smoothAnimationChanged)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration NOTIFY
                   animationDurationChanged)
    Q_PROPERTY(bool showSeconds READ showSeconds WRITE setShowSeconds NOTIFY showSecondsChanged)

public:
    explicit ClockWidget(QWidget *parent = nullptr);
    ~ClockWidget() override;

    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    // 颜色设置
    void setBorderColor(const QColor &color);
    [[nodiscard]] auto borderColor() const -> QColor;

    void setBackgroundColor(const QColor &color);
    [[nodiscard]] auto backgroundColor() const -> QColor;

    void setForegroundColor(const QColor &color);
    [[nodiscard]] auto foregroundColor() const -> QColor;

    void setHourColor(const QColor &color);
    [[nodiscard]] auto hourColor() const -> QColor;

    void setMinuteColor(const QColor &color);
    [[nodiscard]] auto minuteColor() const -> QColor;

    void setSecondColor(const QColor &color);
    [[nodiscard]] auto secondColor() const -> QColor;

    void setTextColor(const QColor &color);
    [[nodiscard]] auto textColor() const -> QColor;

    // 动画设置
    void setSmoothAnimation(bool enabled);
    [[nodiscard]] bool smoothAnimation() const;

    void setAnimationDuration(int duration);
    [[nodiscard]] auto animationDuration() const -> int;

    // 显示设置
    void setShowSeconds(bool show);
    [[nodiscard]] bool showSeconds() const;

public slots:
    void updateTime();
    void startSmoothAnimation();
    void stopSmoothAnimation();
    void toggleSmoothAnimation();

signals:
    void borderColorChanged(const QColor &color);
    void backgroundColorChanged(const QColor &color);
    void foregroundColorChanged(const QColor &color);
    void hourColorChanged(const QColor &color);
    void minuteColorChanged(const QColor &color);
    void secondColorChanged(const QColor &color);
    void textColorChanged(const QColor &color);
    void smoothAnimationChanged(bool enabled);
    void animationDurationChanged(int duration);
    void showSecondsChanged(bool show);
    void timeUpdated(const QTime &time);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onSmoothAnimationTimeout();

private:
    void drawBorder(QPainter *painter);
    void drawBackground(QPainter *painter);
    void drawScale(QPainter *painter);
    void drawScaleNumbers(QPainter *painter);
    void drawHourHand(QPainter *painter);
    void drawMinuteHand(QPainter *painter);
    void drawSecondHand(QPainter *painter);
    void drawCenterDot(QPainter *painter);
    void updateGeometry();

    class ClockWidgetPrivate;
    QScopedPointer<ClockWidgetPrivate> d_ptr;
};
