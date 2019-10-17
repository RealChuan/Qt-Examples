#ifndef ACCEPTWIDGET_H
#define ACCEPTWIDGET_H

#include <QListWidget>

class acceptWidget:public QListWidget
{
    Q_OBJECT
public:
    explicit acceptWidget(QWidget *parent=nullptr);

    void deleteitem();

private:
    QMenu *ListMenuButton;
};

#endif // ACCEPTWIDGET_H
