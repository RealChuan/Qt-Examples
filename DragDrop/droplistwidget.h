#ifndef DROPLISTWIDGET_H
#define DROPLISTWIDGET_H

#include <QListWidget>

class DropListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit DropListWidget(QWidget *parent = nullptr);

private:
    void setupUI();

    QMenu *m_menu;
};

#endif // DROPLISTWIDGET_H
