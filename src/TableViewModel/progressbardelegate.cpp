#include "progressbardelegate.h"

#include <QtWidgets>

ProgressBarDelegate::ProgressBarDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

ProgressBarDelegate::~ProgressBarDelegate() {}

void ProgressBarDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    auto copyOption = option;
    initStyleOption(&copyOption, index);

    const auto *widget = option.widget;
    auto *style = widget ? widget->style() : QApplication::style();

    auto rect = option.rect;
    auto margin = qMin(rect.width(), rect.height()) / 10.0;

    QStyleOptionProgressBar oprtionProgressBar;
    if (widget) {
        oprtionProgressBar.initFrom(widget);
    }
    oprtionProgressBar.state = QStyle::StateFlag::State_Horizontal;
    oprtionProgressBar.direction = copyOption.direction;
    oprtionProgressBar.rect = rect.adjusted(margin, margin, -margin, -margin);
    oprtionProgressBar.fontMetrics = copyOption.fontMetrics;
    oprtionProgressBar.palette = copyOption.palette;

    oprtionProgressBar.minimum = 0;
    oprtionProgressBar.maximum = 100;
    oprtionProgressBar.progress = index.model()->data(index).toUInt();
    oprtionProgressBar.text = tr("%1%").arg(oprtionProgressBar.progress);
    oprtionProgressBar.textAlignment = copyOption.displayAlignment;
    oprtionProgressBar.textVisible = true;

    painter->save();
    style->drawControl(QStyle::CE_ProgressBar, &oprtionProgressBar, painter, widget);
    painter->restore();
}
