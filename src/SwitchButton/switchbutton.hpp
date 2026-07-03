#pragma once

#include <QAbstractButton>

#include <memory>

class SwitchButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(double offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(
        QColor checkedColor READ checkedColor WRITE setCheckedColor NOTIFY checkedColorChanged)
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
    [[nodiscard]] auto sizeHint() const -> QSize override;

    // Color properties
    void setCheckedColor(const QColor &color);
    [[nodiscard]] auto checkedColor() const -> QColor;

    void setUncheckedColor(const QColor &color);
    [[nodiscard]] auto uncheckedColor() const -> QColor;

    void setThumbColor(const QColor &color);
    [[nodiscard]] auto thumbColor() const -> QColor;

    void setThumbBorderColor(const QColor &color);
    [[nodiscard]] auto thumbBorderColor() const -> QColor;

    // Animation
    void setAnimationDuration(int duration);
    [[nodiscard]] auto animationDuration() const -> int;
    [[nodiscard]] bool isAnimating() const;

signals:
    void offsetChanged(double offset);
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

private:
    [[nodiscard]] auto offset() const -> double;
    void setOffset(double offset);
    void updateThumbPosition();
    void startAnimation(bool checked);
    void drawBackground(QPainter &painter, const QRectF &rect);
    void drawThumb(QPainter &painter, const QRectF &rect);
    [[nodiscard]] auto widthMargin() const -> double;
    [[nodiscard]] auto heightMargin() const -> double;
    [[nodiscard]] auto thumbSize() const -> double;
    [[nodiscard]] auto slotRect() const -> QRectF;
    [[nodiscard]] auto thumbRect() const -> QRectF;

    class SwitchButtonPrivate;
    std::unique_ptr<SwitchButtonPrivate> d_ptr;
};
