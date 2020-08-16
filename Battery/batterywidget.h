#ifndef BATTERYWIDGET_H
#define BATTERYWIDGET_H

#include <QWidget>

struct BatteryWidgetPrivate;
class BatteryWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
    Q_PROPERTY(QColor powerColor READ powerColor WRITE setPowerColor)
    Q_PROPERTY(QColor alarmColor READ alarmColor WRITE setAlarmColor)
public:
    explicit BatteryWidget(QWidget *parent = nullptr);
    ~BatteryWidget();

    void setBorderColor(const QColor& color);
    QColor const borderColor() const;

    void setPowerColor(const QColor& color);
    QColor const powerColor() const;

    void setAlarmColor(const QColor& color);
    QColor const alarmColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void valueChanged(const int value);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onStartAnimation(const int value);
    int value() const;
    void setValue(const int value);

private:
    QScopedPointer<BatteryWidgetPrivate> d;
};

#endif // BATTERYWIDGET_H
