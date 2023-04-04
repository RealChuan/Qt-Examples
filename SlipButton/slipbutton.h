#ifndef SLIPBUTTON_H
#define SLIPBUTTON_H

#include <QAbstractButton>

struct SlipButtonPrivate;
class SlipButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(int offset READ offset WRITE setOffset)
    Q_PROPERTY(
        QColor checkedBackgroundColor READ checkedBackgroundColor WRITE setCheckedBackgroundColor)
    Q_PROPERTY(QColor uncheckedBackgroundColor READ uncheckedBackgroundColor WRITE
                   setUncheckedBackgroundColor)
public:
    explicit SlipButton(QWidget *parent = nullptr);
    ~SlipButton() override;

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

public slots:
    void setCheckedBackgroundColor(const QColor &color);
    [[nodiscard]] auto checkedBackgroundColor() const -> QColor;

    void setUncheckedBackgroundColor(const QColor &color);
    [[nodiscard]] auto uncheckedBackgroundColor() const -> QColor;

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onStartAnimation();
    [[nodiscard]] auto offset() const -> int;
    void setOffset(int offset);

private:
    [[nodiscard]] inline auto widthMargin() const -> double;
    [[nodiscard]] inline auto heightMargin() const -> double;

    QScopedPointer<SlipButtonPrivate> d;
};

#endif // SLIPBUTTON_H
