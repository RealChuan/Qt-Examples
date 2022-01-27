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
    Q_PROPERTY(double value READ value WRITE setValue CONSTANT)
    Q_PROPERTY(bool percent READ percent WRITE setPercent CONSTANT)
    Q_PROPERTY(double min READ min WRITE setMin CONSTANT)
    Q_PROPERTY(double max READ max WRITE setmax CONSTANT)
    Q_PROPERTY(double radius READ radius WRITE setRadius CONSTANT)
    Q_PROPERTY(double autoRadius READ autoRadius WRITE setAutoRadius CONSTANT)
    Q_PROPERTY(QColor chunkColor READ chunkColor WRITE setChunkColor CONSTANT)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor CONSTANT)
    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor CONSTANT)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor CONSTANT)
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
