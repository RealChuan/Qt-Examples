#pragma once

#include <QAbstractButton>

class SwitchButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(QColor checkedColor READ checkedColor WRITE setCheckedColor NOTIFY checkedColorChanged)
    Q_PROPERTY(QColor uncheckedColor READ uncheckedColor WRITE setUncheckedColor NOTIFY
                   uncheckedColorChanged)
    Q_PROPERTY(QColor thumbColor READ thumbColor WRITE setThumbColor NOTIFY thumbColorChanged)
    Q_PROPERTY(QColor thumbBorderColor READ thumbBorderColor WRITE setThumbBorderColor NOTIFY
                   thumbBorderColorChanged)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration NOTIFY
                   animationDurationChanged)

public:
    explicit SwitchButton(QWidget *parent = nullptr);
    ~SwitchButton() override;

    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    // 颜色设置
    void setCheckedColor(const QColor &color);
    [[nodiscard]] auto checkedColor() const -> QColor;

    void setUncheckedColor(const QColor &color);
    [[nodiscard]] auto uncheckedColor() const -> QColor;

    void setThumbColor(const QColor &color);
    [[nodiscard]] auto thumbColor() const -> QColor;

    void setThumbBorderColor(const QColor &color);
    [[nodiscard]] auto thumbBorderColor() const -> QColor;

    // 动画设置
    void setAnimationDuration(int duration);
    [[nodiscard]] auto animationDuration() const -> int;
    [[nodiscard]] bool isAnimating() const;

signals:
    void offsetChanged(int offset);
    void checkedColorChanged(const QColor &color);
    void uncheckedColorChanged(const QColor &color);
    void thumbColorChanged(const QColor &color);
    void thumbBorderColorChanged(const QColor &color);
    void animationDurationChanged(int duration);
    void animationStarted(bool checked);
    void animationFinished(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onToggled(bool checked);
    void onAnimationFinished();
    [[nodiscard]] auto offset() const -> int;
    void setOffset(int offset);

private:
    void updateThumbPosition();
    void startAnimation(bool checked);
    void drawBackground(QPainter *painter, const QRectF &rect);
    void drawThumb(QPainter *painter, const QRectF &rect);
    [[nodiscard]] inline auto widthMargin() const -> double;
    [[nodiscard]] inline auto heightMargin() const -> double;
    [[nodiscard]] inline auto thumbSize() const -> double;
    [[nodiscard]] inline auto slotRect() const -> QRectF;
    [[nodiscard]] inline auto thumbRect() const -> QRectF;

    class SwitchButtonPrivate;
    QScopedPointer<SwitchButtonPrivate> d_ptr;
};
