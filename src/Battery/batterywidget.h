#pragma once

#include <QWidget>

class BatteryWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
    Q_PROPERTY(QColor powerColor READ powerColor WRITE setPowerColor)
    Q_PROPERTY(QColor alarmColor READ alarmColor WRITE setAlarmColor)
    Q_PROPERTY(int alarmValue READ alarmValue WRITE setAlarmValue)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration)
    Q_PROPERTY(bool charging READ isCharging WRITE setCharging NOTIFY chargingChanged)

public:
    explicit BatteryWidget(QWidget *parent = nullptr);
    ~BatteryWidget() override;

    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    // 颜色设置
    void setBorderColor(const QColor &color);
    [[nodiscard]] auto borderColor() const -> QColor;

    void setPowerColor(const QColor &color);
    [[nodiscard]] auto powerColor() const -> QColor;

    void setAlarmColor(const QColor &color);
    [[nodiscard]] auto alarmColor() const -> QColor;

    // 数值设置
    void setAlarmValue(int value);
    [[nodiscard]] auto alarmValue() const -> int;

    void setValueAnimated(int value);
    void setValue(int value);
    [[nodiscard]] auto value() const -> int;

    // 动画设置
    void setAnimationDuration(int duration);
    [[nodiscard]] auto animationDuration() const -> int;
    bool isAnimating() const;

    // 充电状态
    void setCharging(bool charging);
    [[nodiscard]] bool isCharging() const;

public slots:
    // 数值控制
    void increaseValue(int increment = 1);
    void decreaseValue(int decrement = 1);
    void reset();

    // 充电控制
    void startCharging() { setCharging(true); }
    void stopCharging() { setCharging(false); }
    void toggleCharging() { setCharging(!isCharging()); }

signals:
    void valueChanged(int value);
    void valueIncreased(int newValue);
    void valueDecreased(int newValue);
    void valueReset();
    void animationStarted(int oldValue, int newValue);
    void animationFinished(int value);
    void animationStopped(int currentValue);
    void alarmStateChanged(bool isInAlarm);
    void chargingChanged(bool charging);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onAnimationFinished();
    void onValueChanged(int value);

private:
    void drawBorder(QPainter *painter, const QRectF &batteryRect, double linew);
    void drawPower(QPainter *painter, const QRectF &batteryRect, double linew);
    void drawValue(QPainter *painter, const QRectF &batteryRect);
    void drawHeader(QPainter *painter, const QRectF &batteryRect);
    void drawChargingSymbol(QPainter *painter, const QRectF &batteryRect);
    void startAnimation(int targetValue);
    void checkAlarmState();

    class BatteryWidgetPrivate;
    QScopedPointer<BatteryWidgetPrivate> d_ptr;
};
