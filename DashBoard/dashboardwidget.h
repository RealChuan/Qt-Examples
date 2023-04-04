#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>

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
    ~DashBoardWidget() override;

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    [[nodiscard]] auto value() const -> double;
    void setValue(const double value);

    void setMin(const double min);
    [[nodiscard]] auto min() const -> double;

    void setmax(const double max);
    [[nodiscard]] auto max() const -> double;

    void setStartAngle(const double startAngle);
    [[nodiscard]] auto startAngle() const -> double;

    void setEndAngle(const double endAngle);
    [[nodiscard]] auto endAngle() const -> double;

    void setScaleMajor(const int scale);
    [[nodiscard]] auto scaleMajor() const -> int;

    void setScaleMinor(const int scale);
    [[nodiscard]] auto scaleMinor() const -> int;

    void setUnit(const QString &unit);
    [[nodiscard]] auto unit() const -> QString;

    void setText(const QString &text);
    [[nodiscard]] auto text() const -> QString;

    void setArcColor(const QColor &color);
    [[nodiscard]] auto arcColor() const -> QColor;

    void setScaleColor(const QColor &color);
    [[nodiscard]] auto scaleColor() const -> QColor;

    void setPointerColor(const QColor &color);
    [[nodiscard]] auto pointerColor() const -> QColor;

    void setTextColor(const QColor &color);
    [[nodiscard]] auto textColor() const -> QColor;

    void setBackgroundColor(const QColor &color);
    [[nodiscard]] auto backgroundColor() const -> QColor;

signals:
    void valueChanged(const double value);

private slots:
    void onStartAnimation(const double value);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawArc(QPainter *painter);
    void drawScale(QPainter *painter);
    void drawScaleNum(QPainter *painter);
    void drawPointer(QPainter *painter);
    void drawValue(QPainter *painter);

    struct DashBoardWidgetPrivate;
    QScopedPointer<DashBoardWidgetPrivate> d_ptr;
};

#endif // DASHBOARDWIDGET_H
