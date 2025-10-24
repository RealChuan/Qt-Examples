#ifndef CLOCKWIDGET_H
#define CLOCKWIDGET_H

#include <QWidget>

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
    ~ClockWidget() override;

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

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

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void setOffset();

private:
    void drawBorder(QPainter *painter);
    void drawBackground(QPainter *painter);
    void drawScale(QPainter *painter, double linew);
    void drawScaleNum(QPainter *painter);
    void drawHour(QPainter *painter);
    void drawMinute(QPainter *painter);
    void drawSecond(QPainter *painter);
    void drawDot(QPainter *painter);

    class ClockWidgetPrivate;
    QScopedPointer<ClockWidgetPrivate> d_ptr;
};

#endif // CLOCKWIDGET_H
