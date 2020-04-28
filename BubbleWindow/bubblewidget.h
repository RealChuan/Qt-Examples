#ifndef BUBBLEWIDGET_H
#define BUBBLEWIDGET_H

#include <QWidget>

#define SHADOW_WIDTH 15         // 窗口阴影宽度

class BubbleWidgetPrivate;
class BubbleWidget : public QWidget
{
    Q_OBJECT
public:
    enum Direction{ Left, Right, Top, Bottom };

    BubbleWidget(QWidget *parent = nullptr);
    ~BubbleWidget() override;

    void setCentralWidget(QWidget *widget);

    // 设置小三角宽和高;
    void setTriangleInfo(int width, int height);
    void setDerection(Direction);

    void exec();

signals:
    void aboutToClose();

protected:
    void paintEvent(QPaintEvent *) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();

    BubbleWidgetPrivate *d;
};

#endif // BUBBLEWIDGET_H
