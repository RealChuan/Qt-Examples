#include "slipbutton.h"

#include <QPropertyAnimation>
#include <QtWidgets>

struct SlipButton::SlipButtonPrivate
{
    int offset = 0;
    QColor uncheckBackgroundColor = QColor(QLatin1String("#FFE0E0E0"));
    QColor checkedBackgroundColor = QColor(QLatin1String("#4da1ff"));
    bool hover = false;
    QPropertyAnimation *animation;
};

SlipButton::SlipButton(QWidget *parent)
    : QAbstractButton(parent)
    , d_ptr(new SlipButtonPrivate)
{
    d_ptr->animation = new QPropertyAnimation(this, "offset", this);
    setCheckable(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(this, &SlipButton::toggled, this, &SlipButton::onStartAnimation);
}

SlipButton::~SlipButton() {}

QSize SlipButton::sizeHint() const
{
    return {20, 20};
}

QSize SlipButton::minimumSizeHint() const
{
    return {20, 20};
}

void SlipButton::setCheckedBackgroundColor(const QColor &color)
{
    d_ptr->checkedBackgroundColor = color;
    update();
}

QColor SlipButton::checkedBackgroundColor() const
{
    return d_ptr->checkedBackgroundColor;
}

void SlipButton::setUncheckedBackgroundColor(const QColor &color)
{
    d_ptr->uncheckBackgroundColor = color;
    update();
}

QColor SlipButton::uncheckedBackgroundColor() const
{
    return d_ptr->uncheckBackgroundColor;
}

int SlipButton::offset() const
{
    return d_ptr->offset;
}

void SlipButton::setOffset(int offset)
{
    d_ptr->offset = offset;
    update();
}

double SlipButton::widthMargin() const
{
    return width() / 22.0;
}

double SlipButton::heightMargin() const
{
    return height() / 22.0;
}

void SlipButton::paintEvent(QPaintEvent *event)
{
    //qDebug() << offset();
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    double w = width() - widthMargin() * 2;
    double h = height() - heightMargin() * 2;

    // 画背景
    QRectF rectSlot((width() - w) / 2, (height() - h) / 2, w, h);
    double slotRoundness = rectSlot.height() / 2;
    painter.setBrush(d_ptr->uncheckBackgroundColor);
    painter.drawRoundedRect(rectSlot, slotRoundness, slotRoundness);

    // 选中情况下，背景变蓝
    if (isEnabled() && isChecked()) {
        QRectF rectSlotFill = rectSlot.adjusted(0, 0, offset() + h - (width() - widthMargin()), 0);
        painter.setBrush(d_ptr->checkedBackgroundColor);
        painter.drawRoundedRect(rectSlotFill, slotRoundness, slotRoundness);
    }

    QRectF rectThumb = QRectF(offset(), (height() - h) / 2, h, h);
    rectThumb.adjust(2, 2, -2, -2);
    QColor colorThumbBorder = (d_ptr->hover) ? d_ptr->checkedBackgroundColor
                                             : QColor(QLatin1String("#FFA8A8A8"));
    painter.setBrush(colorThumbBorder);
    painter.drawEllipse(rectThumb);

    // 按钮圆点
    QColor colorThumb = isEnabled() ? QColor(Qt::white) : QColor(QLatin1String("#FFE0E0E0"));
    painter.setBrush(colorThumb);
    rectThumb.adjust(1.1, 1.1, -1.1, -1.1);
    painter.drawEllipse(rectThumb);
}

void SlipButton::enterEvent(QEnterEvent *event)
{
    QAbstractButton::enterEvent(event);
    setCursor(Qt::PointingHandCursor);
    d_ptr->hover = true;
}

void SlipButton::leaveEvent(QEvent *event)
{
    QAbstractButton::leaveEvent(event);
    d_ptr->hover = false;
}

void SlipButton::resizeEvent(QResizeEvent *event)
{
    QAbstractButton::resizeEvent(event);
    if (isChecked()) {
        double h = height() - heightMargin() * 2;
        setOffset(width() - widthMargin() - h);
    } else
        setOffset(widthMargin());
}

void SlipButton::onStartAnimation()
{
    double h = height() - heightMargin() * 2;
    double start = widthMargin();
    double end = width() - start - h;

    if (!isChecked())
        qSwap(start, end);

    // 改变参数值（offset = startValue, offset += interval_1...offset += interval_N）
    // 改变offset的同时，不断repaint（update）
    // 直到为目标值（offset = endValue）paint完成；
    d_ptr->animation->setStartValue(start);
    d_ptr->animation->setEndValue(end);
    d_ptr->animation->setDuration(120);
    d_ptr->animation->start();
}
