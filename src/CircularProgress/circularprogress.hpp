#pragma once

#include <QWidget>

#include <memory>

using namespace Qt::StringLiterals;

class CircularProgress : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(double minValue READ minValue WRITE setMinValue NOTIFY minValueChanged)
    Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue NOTIFY maxValueChanged)
    Q_PROPERTY(double startAngle READ startAngle WRITE setStartAngle NOTIFY startAngleChanged)
    Q_PROPERTY(double endAngle READ endAngle WRITE setEndAngle NOTIFY endAngleChanged)
    Q_PROPERTY(bool showPercent READ showPercent WRITE setShowPercent NOTIFY showPercentChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QColor arcColor READ arcColor WRITE setArcColor NOTIFY arcColorChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QColor titleColor READ titleColor WRITE setTitleColor NOTIFY titleColorChanged)
    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY
                   backgroundColorChanged)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration NOTIFY
                   animationDurationChanged)

public:
    explicit CircularProgress(const QString &title = u"Progress"_s, QWidget *parent = nullptr);
    ~CircularProgress() override;

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

    // 显示设置
    void setShowPercent(bool percent);
    [[nodiscard]] auto showPercent() const -> bool;

    void setTitle(const QString &title);
    [[nodiscard]] auto title() const -> QString;

    // 颜色设置
    void setArcColor(const QColor &color);
    [[nodiscard]] auto arcColor() const -> QColor;

    void setTextColor(const QColor &color);
    [[nodiscard]] auto textColor() const -> QColor;

    void setTitleColor(const QColor &color);
    [[nodiscard]] auto titleColor() const -> QColor;

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
    void startAngleChanged(double value);
    void endAngleChanged(double value);
    void showPercentChanged(bool percent);
    void titleChanged(const QString &title);
    void arcColorChanged(const QColor &color);
    void textColorChanged(const QColor &color);
    void titleColorChanged(const QColor &color);
    void baseColorChanged(const QColor &color);
    void backgroundColorChanged(const QColor &color);
    void animationDurationChanged(int duration);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onAnimationFinished();

private:
    void setupFont(QPainter &painter, double minSize, double ratio);
    QRectF getValueRect(double minSize) const;
    QRectF getTitleRect(double minSize) const;
    void drawArc(QPainter &painter, double minSize);
    void drawText(QPainter &painter, double minSize);
    void initAnimations();
    void startAnimation(double targetValue);

    class CircularProgressPrivate;
    std::unique_ptr<CircularProgressPrivate> d_ptr;
};
