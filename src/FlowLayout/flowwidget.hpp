#ifndef FLOWWIDGET_HPP
#define FLOWWIDGET_HPP

#include <QtCore/qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif

class QLabel;

class LabelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LabelWidget(const QString &text, QWidget *parent);

    [[nodiscard]] auto text() const -> QString;

signals:
    void removed();

private:
    QLabel *m_label;
};

class FlowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FlowWidget(QWidget *parent = nullptr);
    ~FlowWidget() override;

    void setItems(const QStringList &list);
    void addItem(const QString &text);

    [[nodiscard]] auto items() const -> QStringList;

private slots:
    void onRemoved();

private:
    class FlowWidgetPrivate;
    QScopedPointer<FlowWidgetPrivate> d_ptr;
};

#endif // FLOWWIDGET_HPP
