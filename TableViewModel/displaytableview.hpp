#pragma once

#include "displaydata.hpp"

#include <QTableView>

class DisplayTableView : public QTableView
{
    Q_OBJECT
public:
    explicit DisplayTableView(QWidget *parent = nullptr);
    ~DisplayTableView() override;

    void setDatas(const QVector<DisplayInfo> &datas);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

signals:
    void insertItem();
    void removeItem();

private:
    class DisplayTableViewPrivate;
    QScopedPointer<DisplayTableViewPrivate> d_ptr;
};
