#ifndef CHECKBOXHEADERVIEW_HPP
#define CHECKBOXHEADERVIEW_HPP

#include <QHeaderView>

class CheckBoxHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit CheckBoxHeaderView(Qt::Orientation orientation = Qt::Horizontal,
                                QWidget *parent = nullptr);
    ~CheckBoxHeaderView() override;

    void setState(int state);

signals:
    void stateChanged(bool);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

private:
    void paintCheckBox(QPainter *painter, const QRect &rect) const;

    class CheckBoxHeaderViewPrivate;
    QScopedPointer<CheckBoxHeaderViewPrivate> d_ptr;
};

#endif // CHECKBOXHEADERVIEW_HPP
