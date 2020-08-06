#ifndef SLIPBUTTON_H
#define SLIPBUTTON_H

#include <QAbstractButton>

struct SlipButtonPrivate;
class SlipButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(int offset READ offset WRITE setOffset)
public:
    explicit SlipButton(QWidget* parent = nullptr);
    ~SlipButton() override;

    void setBrush(const QBrush& brush);
    void setButtonHeight(int height);
    void setMargin(int margin);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void startAnimation();
    int offset() const;
    void setOffset(int offset);

private:
    QScopedPointer<SlipButtonPrivate> d;
};


#endif // SLIPBUTTON_H
