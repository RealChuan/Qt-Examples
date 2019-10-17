#ifndef MENWIDGET_H
#define MENWIDGET_H

#include <QListWidget>

class menWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit menWidget(QWidget *parent = nullptr);
};

#endif // MENWIDGET_H
