#ifndef ICONFLITER_HPP
#define ICONFLITER_HPP

#include "qabstractbutton.h"
#include <QObject>

class IconFliter : public QObject
{
    Q_OBJECT
public:
    explicit IconFliter(QAbstractButton *parent = nullptr);
    ~IconFliter() override;

    void setNormalIcon(const QIcon &icon);
    void setNormalIcon(const QStringList &icons);

    void setHoverIcon(const QIcon &icon);
    void setHoverIcon(const QStringList &icons);

    void setPressedIcon(const QIcon &icon);
    void setPressedIcon(const QStringList &icons);

private slots:
    void onToggle(bool checked);

protected:
    auto eventFilter(QObject *watched, QEvent *event) -> bool override;

private:
    void buildConnect();

    class IconFliterPrivate;
    QScopedPointer<IconFliterPrivate> d_ptr;
};

#endif // ICONFLITER_HPP
