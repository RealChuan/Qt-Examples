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
    Q_PROPERTY(QColor currentBackgroundColor READ currentBackgroundColor WRITE setCurrentBackgroundColor)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor)
public:
    explicit NavigationProgressBar(QWidget *parent = nullptr);
    ~NavigationProgressBar();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setMessageList(const QStringList &list);
    QStringList messageList() const;

    void setStep(const int step);
    int step() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    void setCurrentBackgroundColor(const QColor &color);
    QColor currentBackgroundColor() const;

    void setForegroundColor(const QColor &color);
    QColor foregroundColor() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawBackground(QPainter *painter, const bool ok);
    void drawText(QPainter *painter, const bool ok);

    QScopedPointer<NavigationProgressBarPrivate> d;
};

#endif // NAVIGATIONPROGRESSBAR_H
