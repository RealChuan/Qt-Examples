#pragma once

#include <QGraphicsView>

class Carousel3DView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration)
    Q_PROPERTY(int autoRotationInterval READ autoRotationInterval WRITE setAutoRotationInterval)
    Q_PROPERTY(bool autoRotationEnabled READ isAutoRotationEnabled WRITE setAutoRotationEnabled)

public:
    explicit Carousel3DView(QWidget *parent = nullptr);
    ~Carousel3DView();

    // 动画设置
    void setAnimationDuration(int duration);
    int animationDuration() const;

    // 自动轮播设置
    void setAutoRotationInterval(int interval);
    int autoRotationInterval() const;
    void setAutoRotationEnabled(bool enabled);
    bool isAutoRotationEnabled() const;

public slots:
    void rotateToNext();
    void rotateToPrevious();
    void startAutoRotation();
    void stopAutoRotation();
    void toggleAutoRotation();

private slots:
    void onItemClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    class Carousel3DViewPrivate;
    QScopedPointer<Carousel3DViewPrivate> d_ptr;
};
