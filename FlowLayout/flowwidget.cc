#include "flowwidget.hpp"
#include "flowlayout.h"

#include <QtWidgets>

LabelWidget::LabelWidget(const QString &text, QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("QWidget{background: #EEEEEE;border-radius: 5px;}");
    setFixedHeight(25);
    m_label = new QLabel(text, this);
    m_label->setToolTip(text);

    QToolButton *button = new QToolButton(this);
    button->setIcon(qApp->style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    button->setIconSize(QSize(10, 10));
    button->setStyleSheet("QToolButton{border:none;}");
    connect(button, &QToolButton::clicked, this, &LabelWidget::removed);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 6, 10, 7);
    layout->setSpacing(9);
    layout->addWidget(m_label);
    layout->addWidget(button);
}

QString LabelWidget::text() const
{
    return m_label->text().trimmed();
}

class FlowWidget::FlowWidgetPrivate
{
public:
    FlowWidgetPrivate(QWidget *parent)
        : q_ptr(parent)
    {
        flowLayout = new FlowLayout(q_ptr, 9, 5, 5);
    }

    QWidget *q_ptr;

    FlowLayout *flowLayout;
};

FlowWidget::FlowWidget(QWidget *parent)
    : QWidget{parent}
    , d_ptr(new FlowWidgetPrivate(this))
{
    setStyleSheet("QWidget{background:white;}");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

FlowWidget::~FlowWidget() {}

void FlowWidget::setItems(const QStringList &list)
{
    QList<LabelWidget *> widgets = findChildren<LabelWidget *>();
    qDeleteAll(widgets);

    for (const QString &text : qAsConst(list)) {
        addItem(text);
    }
}

void FlowWidget::addItem(const QString &text)
{
    LabelWidget *widget = new LabelWidget(text, this);
    connect(widget, &LabelWidget::removed, this, &FlowWidget::onRemoved);
    d_ptr->flowLayout->addWidget(widget);
}

QStringList FlowWidget::items() const
{
    QStringList list;
    QList<LabelWidget *> widgets = findChildren<LabelWidget *>();
    for (LabelWidget *widget : qAsConst(widgets)) {
        list.append(widget->text());
    }
    return list;
}

void FlowWidget::onRemoved()
{
    LabelWidget *widget = qobject_cast<LabelWidget *>(sender());
    if (!widget) {
        return;
    }
    d_ptr->flowLayout->removeWidget(widget);
    widget->deleteLater();
}
