#include "switchbutton.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOption>

using namespace Qt::StringLiterals;

class SwitchButton::SwitchButtonPrivate
{
public:
    explicit SwitchButtonPrivate(SwitchButton *q) : q_ptr(q) {}

    SwitchButton *q_ptr;

    // Configurable properties
    QColor checkedColor{77, 161, 255};
    QColor uncheckedColor{220, 220, 220};
    QColor thumbColor{Qt::white};
    QColor thumbBorderColor{180, 180, 180};
    int animationDuration = 120;

    // State
    bool hover = false;
    bool animating = false;

    // Animation
    QPropertyAnimation *animation = nullptr;
    double offset = 0.0;

    // Constants
    static constexpr double WIDTH_MARGIN_RATIO = 1.0 / 22.0;
    static constexpr double HEIGHT_MARGIN_RATIO = 1.0 / 22.0;
    static constexpr double THUMB_BORDER_WIDTH = 2.0;
};

SwitchButton::SwitchButton(QWidget *parent)
    : QAbstractButton(parent), d_ptr(std::make_unique<SwitchButtonPrivate>(this))
{
    d_ptr->animation = new QPropertyAnimation(this, "offset", this);
    d_ptr->animation->setDuration(d_ptr->animationDuration);
    d_ptr->animation->setEasingCurve(QEasingCurve::InOutQuad);

    setCheckable(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    connect(this, &SwitchButton::toggled, this, &SwitchButton::onToggled);
    connect(d_ptr->animation, &QPropertyAnimation::finished, this, [this]() {
        d_ptr->animating = false;
        emit animationFinished(isChecked());
    });

    updateThumbPosition();
}

SwitchButton::~SwitchButton() = default;

auto SwitchButton::minimumSizeHint() const -> QSize
{ return {50, 30}; }

auto SwitchButton::sizeHint() const -> QSize
{ return {60, 30}; }

// --- Color properties ---

void SwitchButton::setCheckedColor(const QColor &color)
{
    if (d_ptr->checkedColor == color)
        return;
    d_ptr->checkedColor = color;
    update();
    emit checkedColorChanged(color);
}

auto SwitchButton::checkedColor() const -> QColor
{ return d_ptr->checkedColor; }

void SwitchButton::setUncheckedColor(const QColor &color)
{
    if (d_ptr->uncheckedColor == color)
        return;
    d_ptr->uncheckedColor = color;
    update();
    emit uncheckedColorChanged(color);
}

auto SwitchButton::uncheckedColor() const -> QColor
{ return d_ptr->uncheckedColor; }

void SwitchButton::setThumbColor(const QColor &color)
{
    if (d_ptr->thumbColor == color)
        return;
    d_ptr->thumbColor = color;
    update();
    emit thumbColorChanged(color);
}

auto SwitchButton::thumbColor() const -> QColor
{ return d_ptr->thumbColor; }

void SwitchButton::setThumbBorderColor(const QColor &color)
{
    if (d_ptr->thumbBorderColor == color)
        return;
    d_ptr->thumbBorderColor = color;
    update();
    emit thumbBorderColorChanged(color);
}

auto SwitchButton::thumbBorderColor() const -> QColor
{ return d_ptr->thumbBorderColor; }

// --- Animation ---

void SwitchButton::setAnimationDuration(int duration)
{
    if (duration < 0 || d_ptr->animationDuration == duration)
        return;
    d_ptr->animationDuration = duration;
    d_ptr->animation->setDuration(duration);
    emit animationDurationChanged(duration);
}

auto SwitchButton::animationDuration() const -> int
{ return d_ptr->animationDuration; }

bool SwitchButton::isAnimating() const
{ return d_ptr->animating; }

// --- Offset property (for QPropertyAnimation) ---

auto SwitchButton::offset() const -> double
{ return d_ptr->offset; }

void SwitchButton::setOffset(double offset)
{
    if (qFuzzyCompare(d_ptr->offset, offset))
        return;
    d_ptr->offset = offset;
    update();
    emit offsetChanged(offset);
}

// --- Events ---

void SwitchButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    drawBackground(painter, slotRect());

    // Draw thumb
    drawThumb(painter, thumbRect());

    // Draw focus indicator
    if (hasFocus()) {
        QStyleOptionFocusRect option;
        option.initFrom(this);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
}

void SwitchButton::enterEvent(QEnterEvent *event)
{
    QAbstractButton::enterEvent(event);
    setCursor(Qt::PointingHandCursor);
    d_ptr->hover = true;
    update();
}

void SwitchButton::leaveEvent(QEvent *event)
{
    QAbstractButton::leaveEvent(event);
    d_ptr->hover = false;
    update();
}

void SwitchButton::resizeEvent(QResizeEvent *event)
{
    QAbstractButton::resizeEvent(event);
    updateThumbPosition();
}

// --- Private methods ---

void SwitchButton::onToggled(bool checked)
{ startAnimation(checked); }

void SwitchButton::updateThumbPosition()
{
    if (!d_ptr->animating) {
        const double pos = isChecked() ? (width() - widthMargin() - thumbSize()) : widthMargin();
        setOffset(pos);
    }
}

void SwitchButton::startAnimation(bool checked)
{
    if (d_ptr->animation->state() == QPropertyAnimation::Running) {
        d_ptr->animation->stop();
    }

    const double widthMarginValue = widthMargin();
    const double thumbSizeValue = thumbSize();

    const double start = d_ptr->offset;
    const double end = checked ? (width() - widthMarginValue - thumbSizeValue) : widthMarginValue;

    d_ptr->animation->setStartValue(start);
    d_ptr->animation->setEndValue(end);

    d_ptr->animating = true;
    emit animationStarted(checked);

    d_ptr->animation->start();
}

void SwitchButton::drawBackground(QPainter &painter, const QRectF &rect)
{
    const double roundness = rect.height() / 2.0;

    // Draw unchecked background (full rounded rect)
    painter.setBrush(d_ptr->uncheckedColor);
    painter.drawRoundedRect(rect, roundness, roundness);

    // Calculate fill ratio based on thumb position
    const double widthMarginValue = widthMargin();
    const double thumbSizeValue = thumbSize();
    const double minPos = widthMarginValue;
    const double maxPos = width() - widthMarginValue - thumbSizeValue;

    double fillRatio = 0.0;
    if (maxPos > minPos) {
        fillRatio = (d_ptr->offset - minPos) / (maxPos - minPos);
        fillRatio = qBound(0.0, fillRatio, 1.0);
    }

    // When not animating, snap to checked state
    if (!d_ptr->animating) {
        fillRatio = isChecked() ? 1.0 : 0.0;
    }

    // Draw checked fill using clipping for clean rounded edges
    if (fillRatio > 0.0) {
        const double minFillWidth = thumbSizeValue;
        const double maxFillWidth = rect.width();
        const double fillWidth = minFillWidth + fillRatio * (maxFillWidth - minFillWidth);

        painter.save();
        painter.setClipRect(rect.x(), rect.y(), fillWidth, rect.height());
        painter.setBrush(d_ptr->checkedColor);
        painter.drawRoundedRect(rect, roundness, roundness);
        painter.restore();
    }
}

void SwitchButton::drawThumb(QPainter &painter, const QRectF &rect)
{
    // Determine border color based on hover state
    QColor borderColor = d_ptr->thumbBorderColor;
    if (d_ptr->hover) {
        borderColor = isChecked() ? d_ptr->checkedColor : d_ptr->thumbBorderColor.darker(120);
    }

    // Draw thumb border (outer ellipse)
    painter.setBrush(borderColor);
    painter.drawEllipse(rect);

    // Draw thumb body (inner ellipse)
    const double border = SwitchButtonPrivate::THUMB_BORDER_WIDTH;
    const QRectF innerRect = rect.adjusted(border, border, -border, -border);
    painter.setBrush(d_ptr->thumbColor);
    painter.drawEllipse(innerRect);
}

// --- Geometry calculations ---

auto SwitchButton::widthMargin() const -> double
{ return width() * SwitchButtonPrivate::WIDTH_MARGIN_RATIO; }

auto SwitchButton::heightMargin() const -> double
{ return height() * SwitchButtonPrivate::HEIGHT_MARGIN_RATIO; }

auto SwitchButton::thumbSize() const -> double
{ return height() - 2 * heightMargin(); }

auto SwitchButton::slotRect() const -> QRectF
{
    const double wm = widthMargin();
    const double hm = heightMargin();
    return {wm, hm, width() - 2 * wm, height() - 2 * hm};
}

auto SwitchButton::thumbRect() const -> QRectF
{
    const double size = thumbSize();
    return {d_ptr->offset, heightMargin(), size, size};
}
