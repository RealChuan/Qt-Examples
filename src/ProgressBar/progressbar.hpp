#pragma once

#include <QWidget>

class ProgressBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(double minValue READ minValue WRITE setMinValue NOTIFY minValueChanged)
    Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue NOTIFY maxValueChanged)
    Q_PROPERTY(double radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(bool autoRadius READ autoRadius WRITE setAutoRadius NOTIFY autoRadiusChanged)
    Q_PROPERTY(bool showPercent READ showPercent WRITE setShowPercent NOTIFY showPercentChanged)
    Q_PROPERTY(QColor chunkColor READ chunkColor WRITE setChunkColor NOTIFY chunkColorChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY
                   backgroundColorChanged)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration NOTIFY
                   animationDurationChanged)

public:
    explicit ProgressBar(QWidget *parent = nullptr);
    ~ProgressBar() override;

    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    // 数值设置
    void setValueAnimated(double value);
    void setValue(double value);
    [[nodiscard]] auto value() const -> double;

    void setMinValue(double min);
    [[nodiscard]] auto minValue() const -> double;

    void setMaxValue(double max);
    [[nodiscard]] auto maxValue() const -> double;

    // 圆角设置
    void setRadius(double radius);
    [[nodiscard]] auto radius() const -> double;

    void setAutoRadius(bool autoRadius);
    [[nodiscard]] auto autoRadius() const -> bool;

    // 显示设置
    void setShowPercent(bool percent);
    [[nodiscard]] auto showPercent() const -> bool;

    // 颜色设置
    void setChunkColor(const QColor &color);
    [[nodiscard]] auto chunkColor() const -> QColor;

    void setTextColor(const QColor &color);
    [[nodiscard]] auto textColor() const -> QColor;

    void setBaseColor(const QColor &color);
    [[nodiscard]] auto baseColor() const -> QColor;

    void setBackgroundColor(const QColor &color);
    [[nodiscard]] auto backgroundColor() const -> QColor;

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

    void minValueChanged(double value);
    void maxValueChanged(double value);
    void radiusChanged(double value);
    void autoRadiusChanged(bool autoRadius);
    void showPercentChanged(bool percent);
    void chunkColorChanged(const QColor &color);
    void textColorChanged(const QColor &color);
    void baseColorChanged(const QColor &color);
    void backgroundColorChanged(const QColor &color);
    void animationDurationChanged(int duration);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onAnimationFinished();

private:
    void drawProgressBar(QPainter &painter);
    void drawProgressChunk(QPainter &painter,
                           const QRectF &baseRect,
                           double progressWidth,
                           double baseRadius);
    void drawText(QPainter &painter);
    void startAnimation(double targetValue);
    void initAnimations();
    void setupFont(QPainter &painter);

    class ProgressBarPrivate;
    QScopedPointer<ProgressBarPrivate> d_ptr;
};
