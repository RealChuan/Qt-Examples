#ifndef PROGRESSBARDELEGATE_H
#define PROGRESSBARDELEGATE_H

#include <QStyledItemDelegate>

class QProgressBar;

class ProgressBarDelegate : public QStyledItemDelegate
{
public:
    explicit ProgressBarDelegate(QObject *parent = nullptr);
    ~ProgressBarDelegate() override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

private:
    QScopedPointer<QProgressBar> m_progressBarPtr;
};

#endif // PROGRESSBARDELEGATE_H
