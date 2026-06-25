#pragma once

#include <QGraphicsView>

#include <memory>

class Carousel3DView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration NOTIFY
                   animationDurationChanged)
    Q_PROPERTY(int autoRotationInterval READ autoRotationInterval WRITE setAutoRotationInterval
                   NOTIFY autoRotationIntervalChanged)
    Q_PROPERTY(bool autoRotationEnabled READ isAutoRotationEnabled WRITE setAutoRotationEnabled
                   NOTIFY autoRotationEnabledChanged)

public:
    explicit Carousel3DView(QWidget *parent = nullptr);
    ~Carousel3DView() override;

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

signals:
    void animationDurationChanged(int duration);
    void autoRotationIntervalChanged(int interval);
    void autoRotationEnabledChanged(bool enabled);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    class Carousel3DViewPrivate;
    std::unique_ptr<Carousel3DViewPrivate> d_ptr;
};
