#pragma once

#include <QWidget>

class DashBoardWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(double minValue READ minValue WRITE setMinValue)
    Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue)
    Q_PROPERTY(double startAngle READ startAngle WRITE setStartAngle)
    Q_PROPERTY(double endAngle READ endAngle WRITE setEndAngle)
    Q_PROPERTY(int scaleMajor READ scaleMajor WRITE setScaleMajor)
    Q_PROPERTY(int scaleMinor READ scaleMinor WRITE setScaleMinor)
    Q_PROPERTY(QString unit READ unit WRITE setUnit)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QColor arcColor READ arcColor WRITE setArcColor)
    Q_PROPERTY(QColor scaleColor READ scaleColor WRITE setScaleColor)
    Q_PROPERTY(QColor pointerColor READ pointerColor WRITE setPointerColor)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor valueColor READ valueColor WRITE setValueColor)
    Q_PROPERTY(QColor titleColor READ titleColor WRITE setTitleColor)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration)

public:
    explicit DashBoardWidget(QWidget *parent = nullptr);
    ~DashBoardWidget() override;

    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    // 数值设置
    void setValueAnimated(double value);
    void setValue(double value);
    [[nodiscard]] auto value() const -> double;

    void setMinValue(double min);
    [[nodiscard]] auto minValue() const -> double;

    void setMaxValue(double max);
    [[nodiscard]] auto maxValue() const -> double;

    // 角度设置
    void setStartAngle(double startAngle);
    [[nodiscard]] auto startAngle() const -> double;

    void setEndAngle(double endAngle);
    [[nodiscard]] auto endAngle() const -> double;

    // 刻度设置
    void setScaleMajor(int scale);
    [[nodiscard]] auto scaleMajor() const -> int;

    void setScaleMinor(int scale);
    [[nodiscard]] auto scaleMinor() const -> int;

    // 文本设置
    void setUnit(const QString &unit);
    [[nodiscard]] auto unit() const -> QString;

    void setTitle(const QString &title);
    [[nodiscard]] auto title() const -> QString;

    // 颜色设置
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

    void setValueColor(const QColor &color);
    [[nodiscard]] auto valueColor() const -> QColor;

    void setTitleColor(const QColor &color);
    [[nodiscard]] auto titleColor() const -> QColor;

    // 动画设置
    void setAnimationDuration(int duration);
    [[nodiscard]] auto animationDuration() const -> int;

    [[nodiscard]] bool isAnimating() const;

public slots:
    void increaseValue(double increment = 1.0);
    void decreaseValue(double decrement = 1.0);
    void reset();

signals:
    void valueChanged(double value);
    void valueIncreased(double newValue);
    void valueDecreased(double newValue);
    void valueReset();
    void animationStarted(double oldValue, double newValue);
    void animationFinished(double value);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onAnimationFinished();

private:
    void setupFont(QPainter *painter, double minSize, double ratio);
    QRectF getTextRect(double minSize, double verticalRatio, double heightRatio) const;
    void drawArc(QPainter *painter, double minSize);
    void drawScale(QPainter *painter, double minSize);
    void drawScaleNumbers(QPainter *painter, double minSize);
    void drawPointer(QPainter *painter, double minSize);
    void drawValue(QPainter *painter, double minSize);
    void drawTitle(QPainter *painter, double minSize);
    void startAnimation(double targetValue);

    class DashBoardWidgetPrivate;
    QScopedPointer<DashBoardWidgetPrivate> d_ptr;
};
