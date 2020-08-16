#ifndef CLOCKWIDGET_H
#define CLOCKWIDGET_H

#include <QWidget>

class ClockWidgetPrivate;
class ClockWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor)
    Q_PROPERTY(QColor hourColor READ hourColor WRITE setHourColor)
    Q_PROPERTY(QColor minuteColor READ minuteColor WRITE setMinuteColor)
    Q_PROPERTY(QColor secondColor READ secondColor WRITE setSecondColor)
public:
    explicit ClockWidget(QWidget *parent = nullptr);
    ~ClockWidget();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setBorderColor(const QColor &color);
    QColor borderColor() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    void setForegroundColor(const QColor &color);
    QColor foregroundColor() const;

    void setHourColor(const QColor &color);
    QColor hourColor() const;

    void setMinuteColor(const QColor &color);
    QColor minuteColor() const;

    void setSecondColor(const QColor &color);
    QColor secondColor() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void setOffset();

private:
    void drawBorder(QPainter *painter);
    void drawBackground(QPainter *painter);
    void drawScale(QPainter *painter, const double linew);
    void drawScaleNum(QPainter *painter);
    void drawHour(QPainter *painter);
    void drawMinute(QPainter *painter);
    void drawSecond(QPainter *painter);
    void drawDot(QPainter *painter);

    QScopedPointer<ClockWidgetPrivate> d;
};

#endif // CLOCKWIDGET_H
