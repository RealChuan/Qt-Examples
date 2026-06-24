#pragma once

#include <QWidget>

class NavigationProgressBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(
        QStringList messageList READ messageList WRITE setMessageList NOTIFY messageListChanged)
    Q_PROPERTY(int step READ step WRITE setStep NOTIFY stepChanged)
    Q_PROPERTY(int maxStep READ maxStep NOTIFY maxStepChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY
                   backgroundColorChanged)
    Q_PROPERTY(QColor currentBackgroundColor READ currentBackgroundColor WRITE
                   setCurrentBackgroundColor NOTIFY currentBackgroundColorChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY
                   foregroundColorChanged)
    Q_PROPERTY(QFont textFont READ textFont WRITE setTextFont NOTIFY textFontChanged)
    Q_PROPERTY(QFont dateFont READ dateFont WRITE setDateFont NOTIFY dateFontChanged)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)

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
    void messageListChanged();
    void maxStepChanged(int maxStep);
    void backgroundColorChanged(const QColor &color);
    void currentBackgroundColorChanged(const QColor &color);
    void foregroundColorChanged(const QColor &color);
    void textFontChanged(const QFont &font);
    void dateFontChanged(const QFont &font);
    void spacingChanged(int spacing);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawCompleteProgress(QPainter &painter);
    void drawBackground(QPainter &painter, bool completed);
    void drawText(QPainter &painter, bool completed);
    void rebuildDateList();
    void invalidateCache();
    void rebuildCache();

    class NavigationProgressBarPrivate;
    QScopedPointer<NavigationProgressBarPrivate> d_ptr;
};
