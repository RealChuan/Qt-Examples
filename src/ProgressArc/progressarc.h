#ifndef PROGRESSARC_H
#define PROGRESSARC_H

#include <QWidget>

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
    ~ProgressArc() override;

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    [[nodiscard]] auto value() const -> double;
    void setValue(const double value);

    void setTitle(const QString &title);
    [[nodiscard]] auto title() const -> QString;

    void setPercent(const bool percent);
    [[nodiscard]] auto percent() const -> bool;

    void setMin(const double min);
    [[nodiscard]] auto min() const -> double;

    void setmax(const double max);
    [[nodiscard]] auto max() const -> double;

    void setStartAngle(const double startAngle);
    [[nodiscard]] auto startAngle() const -> double;

    void setEndAngle(const double endAngle);
    [[nodiscard]] auto endAngle() const -> double;

    void setArcColor(const QColor &color);
    [[nodiscard]] auto arcColor() const -> QColor;

    void setTextColor(const QColor &color);
    [[nodiscard]] auto textColor() const -> QColor;

    void setTitleColor(const QColor &color);
    [[nodiscard]] auto titleColor() const -> QColor;

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
    void drawArc(QPainter *painter);
    void drawText(QPainter *painter);

    struct ProgressArcPrivate;
    QScopedPointer<ProgressArcPrivate> d_ptr;
};

#endif // PROGRESSARC_H
