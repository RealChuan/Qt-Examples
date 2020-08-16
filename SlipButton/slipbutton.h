#ifndef SLIPBUTTON_H
#define SLIPBUTTON_H

#include <QAbstractButton>

struct SlipButtonPrivate;
class SlipButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(int offset READ offset WRITE setOffset)
    Q_PROPERTY(QColor checkedBackgroundColor READ checkedBackgroundColor WRITE setCheckedBackgroundColor)
    Q_PROPERTY(QColor uncheckedBackgroundColor READ uncheckedBackgroundColor WRITE setUncheckedBackgroundColor)
public:
    explicit SlipButton(QWidget* parent = nullptr);
    ~SlipButton() override;

    void setCheckedBackgroundColor(const QColor &color);
    QColor checkedBackgroundColor() const;

    void setUncheckedBackgroundColor(const QColor &color);
    QColor uncheckedBackgroundColor() const;

    void setButtonHeight(int height);
    void setMargin(int margin);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onStartAnimation();
    int offset() const;
    void setOffset(int offset);

private:
    inline const QSize constSize() const;

    QScopedPointer<SlipButtonPrivate> d;
};

#endif // SLIPBUTTON_H
