#include "slipbutton.h"

#include <QPropertyAnimation>
#include <QtWidgets>

struct SlipButtonPrivate{
    int height = 18;
    int offset = height / 2;
    int margin = 2;
    QBrush brush = QColor("#4da1ff");
    QPropertyAnimation *animation;
    bool hover = false;
};

SlipButton::SlipButton(QWidget *parent)
    : QAbstractButton(parent)
    , d(new SlipButtonPrivate)
{
    d->animation = new QPropertyAnimation(this, "offset", this);
    setCheckable(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(this, &SlipButton::toggled, this, &SlipButton::startAnimation);
}

SlipButton::~SlipButton()
{
}

QSize SlipButton::sizeHint() const
{
    return QSize(int(d->height * 1.8), d->height);
}

void SlipButton::setBrush(const QBrush& brush)
{
    d->brush = brush;
    update();
}

void SlipButton::setButtonHeight(int height)
{
    d->height = qMax(0, height);
    startAnimation();
}

void SlipButton::setMargin(int margin)
{
    d->margin = qMin(qMax(0, margin), d->height / 2 -1);
    update();
}

int SlipButton::offset() const
{
    return d->offset;
}

void SlipButton::setOffset(int offset)
{
    d->offset = offset;
    update();
}

void SlipButton::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);

    int width = sizeHint().width();
    int height = sizeHint().height();
    const double radius = height / 2;
    QRectF rectSlot = QRectF(0, 0, width, height);
    //QRectF rectSlot = QRectF(0.5, d->margin, width - 0.5, height - d->margin * 2);
    QRectF rectSlotFill = rectSlot.adjusted(0, 0, -(width - offset()) + radius, 0);
    QRectF rectThumb = QRectF(offset() - radius, 0, height, height);
    QColor colorSlot = QColor("#FFE0E0E0");
    QColor colorSlotFill = d->brush.color();
    QColor colorThumbBorder = (d->hover) ? colorSlotFill: QColor("#FFA8A8A8");
    QColor colorThumb = isEnabled() ? QColor(Qt::white) : QColor("#FFE0E0E0");
    double slotRoundness = rectSlot.height() / 2;

    p.setBrush(colorSlot);
    p.drawRoundedRect(rectSlot, slotRoundness, slotRoundness);
    if(isEnabled() && isChecked()) {
        p.setBrush(colorSlotFill);
        p.drawRoundedRect(rectSlotFill, slotRoundness, slotRoundness);
    }

    p.setBrush(colorThumbBorder);
    p.drawEllipse(rectThumb);
    p.setBrush(colorThumb);
    rectThumb.adjust(1.1, 1.1, -1.1, -1.1);
    p.drawEllipse(rectThumb);
}

void SlipButton::enterEvent(QEvent* event)
{
    QAbstractButton::enterEvent(event);
    setCursor(Qt::PointingHandCursor);
    d->hover = true;
}

void SlipButton::leaveEvent(QEvent* event)
{
    QAbstractButton::leaveEvent(event);
    d->hover = false;
}

void SlipButton::startAnimation()
{
    int start = d->height / 2;
    int end = sizeHint().width() - d->height / 2;
    if(!isChecked())
        qSwap(start, end);

    d->animation->setStartValue(start);
    d->animation->setEndValue(end);
    d->animation->setDuration(120);
    d->animation->start();
}
