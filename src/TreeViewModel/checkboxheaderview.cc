#include "checkboxheaderview.hpp"

#include <QtWidgets>

class CheckBoxHeaderView::CheckBoxHeaderViewPrivate
{
public:
    CheckBoxHeaderViewPrivate(QWidget *parent)
        : owner(parent)
    {
        checkBox = new QCheckBox(owner);
        checkBox->setVisible(false);
    }

    QWidget *owner;

    QCheckBox *checkBox;
    bool checked = false;
    bool pressed = false;
    bool tristate = false;
    bool noChange = false;
    QRect checkBoxRect = QRect(20, 6, 18, 18);
};

CheckBoxHeaderView::CheckBoxHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
    , d_ptr(new CheckBoxHeaderViewPrivate(this))
{
    setMouseTracking(true);
    setSectionsClickable(true);
    setDefaultAlignment(Qt::AlignVCenter);
    setSortIndicatorShown(false);
    setFixedHeight(30);
    setStyleSheet(
        "QHeaderView{background: #F4F7FC;color: #222222;border-top: 1px solid#F7F7F7;}"
        "QHeaderView::section:first{background: #F4F7FC;padding-left:50px;color: #222222;}"
        "QHeaderView::section{border: 1px solid #F7F7F7;background: "
        "#F4F7FC;padding-left:10px;color: #222222;}");
}

CheckBoxHeaderView::~CheckBoxHeaderView() {}

void CheckBoxHeaderView::setState(int state)
{
    if (state == Qt::PartiallyChecked) {
        d_ptr->tristate = true;
        d_ptr->noChange = true;
    } else {
        d_ptr->noChange = false;
    }

    d_ptr->checked = (state != Qt::Unchecked);
    updateSection(0);
}

void CheckBoxHeaderView::mousePressEvent(QMouseEvent *event)
{
    int index = logicalIndexAt(event->pos());
    if (index == 0 && event->button() & Qt::LeftButton
        && d_ptr->checkBoxRect.contains(event->pos())) {
        d_ptr->pressed = true;
        return;
    }
    QHeaderView::mousePressEvent(event);
}

void CheckBoxHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
    if (d_ptr->pressed) {
        if (d_ptr->tristate && d_ptr->noChange) {
            d_ptr->checked = true;
            d_ptr->noChange = false;
        } else {
            d_ptr->checked = !d_ptr->checked;
        }

        updateSection(0);

        Qt::CheckState state;
        state = d_ptr->checked ? Qt::Checked : Qt::Unchecked;

        emit stateChanged(state);
    } else {
        QHeaderView::mouseReleaseEvent(event);
    }

    d_ptr->pressed = false;
}

void CheckBoxHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    if (logicalIndex == 0) {
        paintCheckBox(painter, rect);
    }
}

void CheckBoxHeaderView::paintCheckBox(QPainter *painter, const QRect &rect) const
{
    QStyleOptionButton option;
    option.initFrom(this);

    if (d_ptr->checked)
        option.state |= QStyle::State_Sunken;

    if (d_ptr->tristate && d_ptr->noChange)
        option.state |= QStyle::State_NoChange;
    else
        option.state |= d_ptr->checked ? QStyle::State_On : QStyle::State_Off;

    //QCheckBox checkBox;
    option.iconSize = QSize(18, 18);
    option.rect = QRect(rect.x() + 20, 6, 18, 18);
    d_ptr->checkBoxRect = option.rect;
    style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter, d_ptr->checkBox);
}
