#ifndef PROGRESSARC_H
#define PROGRESSARC_H

#include <QWidget>

struct ProgressArcPrivate;
class ProgressArc : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool percent READ percent WRITE setPercent)
    Q_PROPERTY(double min READ min WRITE setMin)
    Q_PROPERTY(double max READ max WRITE setmax)
    Q_PROPERTY(double startAngle READ startAngle WRITE setStartAngle)
    Q_PROPERTY(double endAngle READ endAngle WRITE setEndAngle)
    Q_PROPERTY(QColor arcColor READ arcColor WRITE setArcColor)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QColor titleColor READ titleColor WRITE setTitleColor)
    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
public:
    explicit ProgressArc(const QString &title, QWidget *parent = nullptr);
    ~ProgressArc();

    void setTitle(const QString &title);
    QString const title() const;

    void setPercent(const bool percent);
    bool percent() const;

    void setMin(const double min);
    double min() const;

    void setmax(const double max);
    double max() const;

    void setStartAngle(const double startAngle);
    double startAngle() const;

    void setEndAngle(const double endAngle);
    double endAngle() const;

    void setArcColor(const QColor &color);
    QColor const arcColor() const;

    void setTextColor(const QColor &color);
    QColor const textColor() const;

    void setTitleColor(const QColor &color);
    QColor const titleColor() const;

    void setBaseColor(const QColor &color);
    QColor const baseColor() const;

    void setBackgroundColor(const QColor &color);
    QColor const backgroundColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void valueChanged(const double value);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onStartAnimation(const double value);
    double value() const;
    void setValue(const double value);

private:
    void drawArc(QPainter *painter);
    void drawText(QPainter *painter);

    QScopedPointer<ProgressArcPrivate> d;
};

#endif // PROGRESSARC_H
