#pragma once

#include <QWidget>

class NavigationProgressBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList messageList READ messageList WRITE setMessageList)
    Q_PROPERTY(int step READ step WRITE setStep)
    Q_PROPERTY(int maxStep READ maxStep)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(
        QColor currentBackgroundColor READ currentBackgroundColor WRITE setCurrentBackgroundColor)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor)
    Q_PROPERTY(QFont textFont READ textFont WRITE setTextFont)
    Q_PROPERTY(QFont dateFont READ dateFont WRITE setDateFont)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)

public:
    explicit NavigationProgressBar(QWidget *parent = nullptr);
    ~NavigationProgressBar() override;

    QSize minimumSizeHint() const override;

    void setMessageList(const QStringList &list);
    QStringList messageList() const;

    void setStep(int step);
    int step() const;
    int maxStep() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    void setCurrentBackgroundColor(const QColor &color);
    QColor currentBackgroundColor() const;

    void setForegroundColor(const QColor &color);
    QColor foregroundColor() const;

    void setTextFont(const QFont &font);
    QFont textFont() const;

    void setDateFont(const QFont &font);
    QFont dateFont() const;

    void setSpacing(int spacing);
    int spacing() const;

    QString dateAt(int step) const;
    void setDateAt(int step, const QString &date);

public slots:
    void reset();
    void next();
    void previous();

signals:
    void stepChanged(int step);
    void progressCompleted();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawCompleteProgress(QPainter *painter);
    void drawBackground(QPainter *painter, bool completed);
    void drawText(QPainter *painter, bool completed);
    void rebuildDateList();
    void invalidateCache();
    void rebuildCache();

    class NavigationProgressBarPrivate;
    QScopedPointer<NavigationProgressBarPrivate> d_ptr;
};
