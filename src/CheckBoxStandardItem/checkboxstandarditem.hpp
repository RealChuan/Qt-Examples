#ifndef CHECKBOXSTANDARDITEM_HPP
#define CHECKBOXSTANDARDITEM_HPP

#include <QStandardItem>

class CheckBoxStandardItem : public QStandardItem
{
public:
    explicit CheckBoxStandardItem(const QString &text);

    void setData(const QVariant &value, int role = Qt::UserRole + 1) override;
};

#endif // CHECKBOXSTANDARDITEM_HPP
