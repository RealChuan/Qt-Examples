#include "slipbutton.h"

#include <QPropertyAnimation>
#include <QtWidgets>

#define ScaleFactor 1.8

struct SlipButtonPrivate{
    int height = 18;
    int offset = height / 2;
    int margin = 2;
    QColor uncheckBackgroundColor = QColor("#FFE0E0E0");
    QColor checkedBackgroundColor = QColor("#4da1ff");
    bool hover = false;
    bool constSize = false;
    QPropertyAnimation *animation;
};

SlipButton::SlipButton(QWidget *parent)
    : QAbstractButton(parent)
    , d(new SlipButtonPrivate)
{
    d->animation = new QPropertyAnimation(this, "offset", this);
    setCheckable(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(this, &SlipButton::toggled, this, &SlipButton::onStartAnimation);
}

SlipButton::~SlipButton()
{
}

QSize SlipButton::sizeHint() const
{
    if(d->constSize)
        return constSize();
    return QSize(180, 100);
}

QSize SlipButton::minimumSizeHint() const
{
    if(d->constSize)
        return constSize();
    return QSize(18 * ScaleFactor, 18);
}

void SlipButton::setCheckedBackgroundColor(const QColor& color)
{
    d->checkedBackgroundColor = color;
    update();
}

QColor SlipButton::checkedBackgroundColor() const
{
    return d->checkedBackgroundColor;
}

void SlipButton::setUncheckedBackgroundColor(const QColor &color)
{
    d->uncheckBackgroundColor = color;
    update();
}

QColor SlipButton::uncheckedBackgroundColor() const
{
    return d->uncheckBackgroundColor;
}

void SlipButton::setButtonHeight(int height)
{
    d->constSize = true;
    d->height = qMax(d->height, height);
    onStartAnimation();
}

void SlipButton::setMargin(int margin)
{
    d->margin = qMin(qMax(0, margin), height() / 2 - 1);
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

const QSize SlipButton::constSize() const
{
    return QSize(d->height * ScaleFactor, d->height);
}

void SlipButton::paintEvent(QPaintEvent* event)
{
    //qDebug() << offset();
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = - d->margin;
    int h = - d->margin;
    if(d->constSize){
        w += constSize().width();
        h += constSize().height();
    }else{
        w += width();
        h += height();
    }

    // 画背景
    QRectF rectSlot = QRectF(0, 0, w, h);
    double slotRoundness = rectSlot.height() / 2;
    painter.setBrush(d->uncheckBackgroundColor);
    painter.drawRoundedRect(rectSlot, slotRoundness, slotRoundness);

    // 选中情况下，背景变蓝
    const double radius = h / 2;
    if(isEnabled() && isChecked()){
        QRectF rectSlotFill = rectSlot.adjusted(0, 0, -(w - offset()) + radius, 0);
        painter.setBrush(d->checkedBackgroundColor);
        painter.drawRoundedRect(rectSlotFill, slotRoundness, slotRoundness);
    }

    // hover 边框
    QRectF rectThumb = QRectF(offset() - radius, 0, h, h);
    QColor colorThumbBorder = (d->hover) ? d->checkedBackgroundColor: QColor("#FFA8A8A8");
    painter.setBrush(colorThumbBorder);
    painter.drawEllipse(rectThumb);

    // 按钮圆点
    QColor colorThumb = isEnabled() ? QColor(Qt::white) : QColor("#FFE0E0E0");
    painter.setBrush(colorThumb);
    rectThumb.adjust(1.1, 1.1, -1.1, -1.1);
    painter.drawEllipse(rectThumb);
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

void SlipButton::resizeEvent(QResizeEvent *event)
{
    if(d->constSize)
        return;
    int h = event->size().height();
    int w = event->size().width();
    if(w < ScaleFactor * h)
        h = w / ScaleFactor;
    resize(h * ScaleFactor, h);
    onStartAnimation();
}

void SlipButton::onStartAnimation()
{
    int start = 0;
    int end = 0;
    if(d->constSize){
        start = d->height / 2;
        end = constSize().width() - d->height / 2;
    }else{
        start = height() / 2;
        end = width() - height() / 2;
    }

    if(!isChecked())
        qSwap(start, end);

    // 改变参数值（offset = startValue, offset += interval_1...offset += interval_N）
    // 改变offset的同时，不断repaint（update）
    // 直到为目标值（offset = endValue）paint完成；
    d->animation->setStartValue(start);
    d->animation->setEndValue(end);
    d->animation->setDuration(120);
    d->animation->start();
}
