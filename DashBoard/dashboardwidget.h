#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>

struct DashBoardWidgetPrivate;
class DashBoardWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue)
    Q_PROPERTY(double min READ min WRITE setMin)
    Q_PROPERTY(double max READ max WRITE setmax)
    Q_PROPERTY(double startAngle READ startAngle WRITE setStartAngle)
    Q_PROPERTY(double endAngle READ endAngle WRITE setEndAngle)
    Q_PROPERTY(int scaleMajor READ scaleMajor WRITE setScaleMajor)
    Q_PROPERTY(int scaleMinor READ scaleMinor WRITE setScaleMinor)
    Q_PROPERTY(QString unit READ unit WRITE setUnit)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QColor arcColor READ arcColor WRITE setArcColor)
    Q_PROPERTY(QColor scaleColor READ scaleColor WRITE setScaleColor)
    Q_PROPERTY(QColor pointerColor READ pointerColor WRITE setPointerColor)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
public:
    explicit DashBoardWidget(QWidget *parent = nullptr);
    ~DashBoardWidget();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setMin(const double min);
    double min() const;

    void setmax(const double max);
    double max() const;

    void setStartAngle(const double startAngle);
    double startAngle() const;

    void setEndAngle(const double endAngle);
    double endAngle() const;

    void setScaleMajor(const int scale);
    int scaleMajor() const;

    void setScaleMinor(const int scale);
    int scaleMinor() const;

    void setUnit(const QString &unit);
    QString unit() const;

    void setText(const QString &text);
    QString text() const;

    void setArcColor(const QColor &color);
    QColor arcColor() const;

    void setScaleColor(const QColor &color);
    QColor scaleColor() const;

    void setPointerColor(const QColor &color);
    QColor pointerColor() const;

    void setTextColor(const QColor &color);
    QColor textColor() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

signals:
    void valueChanged(const double value);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onStartAnimation(const double value);
    double value() const;
    void setValue(const double value);

private:
    void drawArc(QPainter *painter);
    void drawScale(QPainter *painter);
    void drawScaleNum(QPainter *painter);
    void drawPointer(QPainter *painter);
    void drawValue(QPainter *painter);

    QScopedPointer<DashBoardWidgetPrivate> d;
};

#endif // DASHBOARDWIDGET_H
