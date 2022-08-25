#ifndef PROGRESSBAR_HPP
#define PROGRESSBAR_HPP

#include <QtCore/qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif

class ProgressBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue)
    Q_PROPERTY(bool percent READ percent WRITE setPercent)
    Q_PROPERTY(double min READ min WRITE setMin)
    Q_PROPERTY(double max READ max WRITE setmax)
    Q_PROPERTY(double radius READ radius WRITE setRadius)
    Q_PROPERTY(double autoRadius READ autoRadius WRITE setAutoRadius)
    Q_PROPERTY(QColor chunkColor READ chunkColor WRITE setChunkColor)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
public:
    explicit ProgressBar(QWidget *parent = nullptr);
    ~ProgressBar();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setValue(const double value);
    double value() const;

    void setPercent(const bool percent);
    bool percent() const;

    void setMin(const double min);
    double min() const;

    void setmax(const double max);
    double max() const;

    void setRadius(const double radius);
    double radius() const;

    void setAutoRadius(bool autoRadius);
    double autoRadius() const;

    void setChunkColor(const QColor &color);
    QColor chunkColor() const;

    void setTextColor(const QColor &color);
    QColor textColor() const;

    void setBaseColor(const QColor &color);
    QColor baseColor() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

signals:
    void valueChanged(const double value);

private slots:
    void onStartAnimation(const double value);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawProgressBar(QPainter *painter);
    void drawText(QPainter *painter);

    struct ProgressBarPrivate;
    QScopedPointer<ProgressBarPrivate> d_ptr;
};

#endif // PROGRESSBAR_HPP
