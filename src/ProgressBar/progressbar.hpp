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
    ~ProgressBar() override;

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    void setValue(const double value);
    [[nodiscard]] auto value() const -> double;

    void setPercent(const bool percent);
    [[nodiscard]] auto percent() const -> bool;

    void setMin(const double min);
    [[nodiscard]] auto min() const -> double;

    void setmax(const double max);
    [[nodiscard]] auto max() const -> double;

    void setRadius(const double radius);
    [[nodiscard]] auto radius() const -> double;

    void setAutoRadius(bool autoRadius);
    [[nodiscard]] auto autoRadius() const -> double;

    void setChunkColor(const QColor &color);
    [[nodiscard]] auto chunkColor() const -> QColor;

    void setTextColor(const QColor &color);
    [[nodiscard]] auto textColor() const -> QColor;

    void setBaseColor(const QColor &color);
    [[nodiscard]] auto baseColor() const -> QColor;

    void setBackgroundColor(const QColor &color);
    [[nodiscard]] auto backgroundColor() const -> QColor;

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
