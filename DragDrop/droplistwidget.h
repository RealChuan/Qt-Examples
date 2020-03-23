#ifndef DROPLISTWIDGET_H
#define DROPLISTWIDGET_H

#include <QListWidget>

class DropListWidget : public QListWidget
{
    Q_OBJECT
public:
    DropListWidget(QWidget *parent = nullptr);

private:
    void setupUI();
    QMenu *menu;
};

#endif // DROPLISTWIDGET_H
