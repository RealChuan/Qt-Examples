#ifndef BUBBLEWIDGET_H
#define BUBBLEWIDGET_H

#include <QWidget>

class BubbleWidget : public QWidget
{
    Q_OBJECT
public:
    enum Direction { Left, Right, Top, Bottom };

    explicit BubbleWidget(QWidget *parent = nullptr);
    ~BubbleWidget() override;

    void setPen(const QPen &pen);
    [[nodiscard]] auto pen() const -> QPen;

    void setBrush(const QBrush &brush);
    [[nodiscard]] auto brush() const -> QBrush;

    void setShadowWidth(qint64 width);
    auto shadowWidth() -> qint64;

    void setBorderRadius(qint64 radius);
    auto borderRadius() -> qint64;

    void setText(const QString &text);
    [[nodiscard]] auto text() const -> QString;

    // 设置小三角宽和高;
    void setTriangleInfo(int width, int height);

    void setDerection(Direction);
    auto direction() -> Direction;

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
