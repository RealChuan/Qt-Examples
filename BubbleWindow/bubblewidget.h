#ifndef BUBBLEWIDGET_H
#define BUBBLEWIDGET_H

#include <QWidget>

class BubbleWidget : public QWidget
{
    Q_OBJECT
public:
    enum Direction{ Left, Right, Top, Bottom };

    explicit BubbleWidget(QWidget *parent = nullptr);
    ~BubbleWidget() override;

    void setPen(const QPen &pen);
    QPen pen() const;

    void setBrush(const QBrush &brush);
    QBrush brush() const;

    void setShadowWidth(qint64 width);
    qint64 shadowWidth();

    void setBorderRadius(qint64 radius);
    qint64 borderRadius();

    void setText(const QString &text);
    QString text() const;

    // 设置小三角宽和高;
    void setTriangleInfo(int width, int height);

    void setDerection(Direction);
    Direction direction();

    void exec();

signals:
    void aboutToClose();

protected:
    void paintEvent(QPaintEvent *) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();

    class BubbleWidgetPrivate;
    QScopedPointer<BubbleWidgetPrivate> d_ptr;
};

#endif // BUBBLEWIDGET_H
