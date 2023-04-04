#ifndef BATTERYWIDGET_H
#define BATTERYWIDGET_H

#include <QWidget>

class BatteryWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
    Q_PROPERTY(QColor powerColor READ powerColor WRITE setPowerColor)
    Q_PROPERTY(QColor alarmColor READ alarmColor WRITE setAlarmColor)
public:
    explicit BatteryWidget(QWidget *parent = nullptr);
    ~BatteryWidget() override;

    [[nodiscard]] [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    void setBorderColor(const QColor &color);
    [[nodiscard]] [[nodiscard]] auto borderColor() const -> QColor;

    void setPowerColor(const QColor &color);
    [[nodiscard]] [[nodiscard]] auto powerColor() const -> QColor;

    void setAlarmColor(const QColor &color);
    [[nodiscard]] [[nodiscard]] auto alarmColor() const -> QColor;

    void setValue(int value);
    [[nodiscard]] [[nodiscard]] auto value() const -> int;

signals:
    void valueChanged(int value);

private slots:
    void onStartAnimation(int value);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawBorder(QPainter *painter, const QRectF &batteryRect, double linew);
    void drawPower(QPainter *painter, const QRectF &batteryRect, double linew);
    void drawValue(QPainter *painter, const QRectF &batteryRect);
    void drawHeader(QPainter *painter, const QRectF &batteryRect);

    struct BatteryWidgetPrivate;
    QScopedPointer<BatteryWidgetPrivate> d_ptr;
};

#endif // BATTERYWIDGET_H
