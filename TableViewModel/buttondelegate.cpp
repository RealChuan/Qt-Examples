#include "buttondelegate.h"
#include "displaydata.hpp"

#include <QtWidgets>

ButtonDelegate::ButtonDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

ButtonDelegate::~ButtonDelegate() {}

void ButtonDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    auto copyOption = option;
    initStyleOption(&copyOption, index);

    const auto *widget = option.widget;
    auto *style = widget ? widget->style() : QApplication::style();

    auto rect = option.rect;
    auto margin = qMin(rect.width(), rect.height()) / 10.0;

    QStyleOptionButton optionButton;
    if (widget) {
        optionButton.initFrom(widget);
    }
    optionButton.state = copyOption.state;
    optionButton.direction = copyOption.direction;
    optionButton.rect = rect.adjusted(margin, margin, -margin, -margin);
    optionButton.fontMetrics = copyOption.fontMetrics;
    optionButton.palette = copyOption.palette;
    optionButton.text = copyOption.text;

    painter->save();
    style->drawControl(QStyle::CE_PushButton, &optionButton, painter, widget);
    painter->restore();
}

bool ButtonDelegate::editorEvent(QEvent *event,
                                 QAbstractItemModel *model,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index)
{
    auto type = event->type();
    switch (event->type()) {
    case QEvent::MouseButtonRelease: {
        auto data = model->data(index, Qt::UserRole).value<DisplayInfo>();
        auto details = tr("Title: %1\nNumber: %2\nState: %3\nProcess: %4\nRichText: %5")
                           .arg(data.title())
                           .arg(data.number())
                           .arg(data.state())
                           .arg(data.process())
                           .arg(data.richText() + "x");

        auto *widget = const_cast<QWidget *>(option.widget);
        if (widget) {
            while (widget->parentWidget()) {
                widget = widget->parentWidget();
            }

            QDialog dialog(widget);
            dialog.setMinimumSize(400, 250);
            auto *label = new QLabel(details, &dialog);
            label->setWordWrap(true);
            auto *layout = new QHBoxLayout(&dialog);
            layout->addWidget(label);
            dialog.exec();
        }
    } break;
    default: break;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
