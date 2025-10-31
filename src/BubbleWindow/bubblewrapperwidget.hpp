#pragma once

#include "bubblewidget.h"

#include <QPropertyAnimation>

class BubbleWrapperWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration)
    Q_PROPERTY(double opacity READ windowOpacity WRITE setWindowOpacity)

public:
    explicit BubbleWrapperWidget(QWidget *parent = nullptr);
    ~BubbleWrapperWidget() override;

    // 获取内部气泡控件
    BubbleWidget *bubbleWidget() const;

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    // 动画设置
    void setAnimationDuration(int duration);
    [[nodiscard]] auto animationDuration() const -> int;
    bool isAnimating() const;

signals:
    void aboutToShow();
    void aboutToClose();
    void animationStarted();
    void animationFinished();
    void animationStopped();

public slots:
    // 显示控制
    void showAt(const QPoint &pos,
                BubbleWidget::Direction direction = BubbleWidget::Direction::Bottom);
    void showNearWidget(QWidget *widget,
                        BubbleWidget::Direction direction = BubbleWidget::Direction::Bottom,
                        int margin = 5);

    // 动画控制
    void showAnimated();
    void hideAnimated();
    void toggleAnimation();
    void stopAnimation();

private slots:
    void onEnsureInitialized();
    void onShowAnimationFinished();
    void onHideAnimationFinished();

protected:
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void setupUI();
    void buildConnect();
    [[nodiscard]] auto getAvailableScreenGeometry() const -> QRect;
    void startShowAnimation();
    void startHideAnimation();

    class BubbleWrapperWidgetPrivate;
    QScopedPointer<BubbleWrapperWidgetPrivate> d_ptr;
};
