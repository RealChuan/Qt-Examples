#include "progressbardelegate.h"

#include <QPainter>
#include <QtWidgets>

void ProgressBarDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    if (option.state.testFlag(QStyle::State_HasFocus))
        viewOption.state = viewOption.state ^ QStyle::State_HasFocus;

    QStyledItemDelegate::paint(painter, viewOption, index);

    int value = index.model()->data(index).toUInt();
    if (value < 0)
        value = 0;
    else if (value > 100)
        value = 100;
    int w = qMin(option.rect.width(), option.rect.height()) / 10.0;
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.initFrom(option.widget);
    progressBarOption.rect = option.rect.adjusted(w, w, -w, -w);
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = true;
    progressBarOption.progress = value;
    progressBarOption.text = tr("%1%").arg(progressBarOption.progress);

    painter->save();
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setBrush(option.palette.highlightedText());
    }

    QProgressBar progressBar; // qss 生效
    qApp->style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter, &progressBar);

    painter->restore();
}
