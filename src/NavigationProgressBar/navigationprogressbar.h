#ifndef NAVIGATIONPROGRESSBAR_H
#define NAVIGATIONPROGRESSBAR_H

#include <QWidget>

class NavigationProgressBarPrivate;
class NavigationProgressBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList messageList READ messageList WRITE setMessageList)
    Q_PROPERTY(int step READ step WRITE setStep)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(
        QColor currentBackgroundColor READ currentBackgroundColor WRITE setCurrentBackgroundColor)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor)
public:
    explicit NavigationProgressBar(QWidget *parent = nullptr);
    ~NavigationProgressBar() override;

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    void setMessageList(const QStringList &list);
    [[nodiscard]] auto messageList() const -> QStringList;

    void setStep(const int step);
    [[nodiscard]] auto step() const -> int;

    void setBackgroundColor(const QColor &color);
    [[nodiscard]] auto backgroundColor() const -> QColor;

    void setCurrentBackgroundColor(const QColor &color);
    [[nodiscard]] auto currentBackgroundColor() const -> QColor;

    void setForegroundColor(const QColor &color);
    [[nodiscard]] auto foregroundColor() const -> QColor;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawBackground(QPainter *painter, const bool ok);
    void drawText(QPainter *painter, const bool ok);

    QScopedPointer<NavigationProgressBarPrivate> d;
};

#endif // NAVIGATIONPROGRESSBAR_H
