#ifndef PROGRESSARC_H
#define PROGRESSARC_H

#include <QWidget>

struct ProgressArcPrivate;
class ProgressArc : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue CONSTANT)
    Q_PROPERTY(QString title READ title WRITE setTitle CONSTANT)
    Q_PROPERTY(bool percent READ percent WRITE setPercent CONSTANT)
    Q_PROPERTY(double min READ min WRITE setMin CONSTANT)
    Q_PROPERTY(double max READ max WRITE setmax CONSTANT)
    Q_PROPERTY(double startAngle READ startAngle WRITE setStartAngle CONSTANT)
    Q_PROPERTY(double endAngle READ endAngle WRITE setEndAngle CONSTANT)
    Q_PROPERTY(QColor arcColor READ arcColor WRITE setArcColor CONSTANT)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor CONSTANT)
    Q_PROPERTY(QColor titleColor READ titleColor WRITE setTitleColor CONSTANT)
    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor CONSTANT)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor CONSTANT)
public:
    explicit ProgressArc(const QString &title, QWidget *parent = nullptr);
    ~ProgressArc();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    double value() const;
    void setValue(const double value);

    void setTitle(const QString &title);
    QString title() const;

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
    QColor arcColor() const;

    void setTextColor(const QColor &color);
    QColor textColor() const;

    void setTitleColor(const QColor &color);
    QColor titleColor() const;

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
    void drawArc(QPainter *painter);
    void drawText(QPainter *painter);

    QScopedPointer<ProgressArcPrivate> d;
};

#endif // PROGRESSARC_H
