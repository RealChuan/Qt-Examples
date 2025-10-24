#include "iconfliter.hpp"

#include <QtWidgets>

auto iconFromPaths(const QStringList &paths) -> QIcon
{
    QIcon icon;
    for (const auto &path : std::as_const(paths)) {
        icon.addFile(path);
    }
    return icon;
}

class IconFliter::IconFliterPrivate
{
public:
    explicit IconFliterPrivate(IconFliter *q)
        : q_ptr(q)
    {}

    IconFliter *q_ptr;

    QPointer<QAbstractButton> buttonPtr;

    QIcon normalIcon;
    QIcon hoverIcon;
    QIcon pressedIcon;
};

IconFliter::IconFliter(QAbstractButton *parent)
    : QObject{parent}
    , d_ptr(new IconFliterPrivate(this))
{
    d_ptr->buttonPtr = parent;
    if (!d_ptr->buttonPtr.isNull()) {
        d_ptr->buttonPtr->installEventFilter(this);
    }
    buildConnect();
}

IconFliter::~IconFliter() = default;

void IconFliter::setNormalIcon(const QIcon &icon)
{
    d_ptr->normalIcon = icon;
    if (!d_ptr->buttonPtr.isNull()) {
        d_ptr->buttonPtr->setIcon(d_ptr->normalIcon);
    }
}

void IconFliter::setNormalIcon(const QStringList &icons)
{
    setNormalIcon(iconFromPaths(icons));
}

void IconFliter::setHoverIcon(const QIcon &icon)
{
    d_ptr->hoverIcon = icon;
}

void IconFliter::setHoverIcon(const QStringList &icons)
{
    setHoverIcon(iconFromPaths(icons));
}

void IconFliter::setPressedIcon(const QIcon &icon)
{
    d_ptr->pressedIcon = icon;
}

void IconFliter::setPressedIcon(const QStringList &icons)
{
    setPressedIcon(iconFromPaths(icons));
}

void IconFliter::onToggle(bool checked)
{
    if (!d_ptr->buttonPtr.isNull()) {
        d_ptr->buttonPtr->setIcon(checked ? d_ptr->pressedIcon : d_ptr->normalIcon);
    }
}

auto IconFliter::eventFilter(QObject *watched, QEvent *event) -> bool
{
    if (!d_ptr->buttonPtr.isNull() && watched == d_ptr->buttonPtr.data()) {
        switch (event->type()) {
        case QEvent::Enter: d_ptr->buttonPtr->setIcon(d_ptr->hoverIcon); break;
        case QEvent::Leave: onToggle(d_ptr->buttonPtr->isChecked()); break;
        case QEvent::MouseButtonPress: d_ptr->buttonPtr->setIcon(d_ptr->pressedIcon); break;
        default: break;
        }
    }
    return QObject::eventFilter(watched, event);
}

void IconFliter::buildConnect()
{
    if (!d_ptr->buttonPtr.isNull()) {
        connect(d_ptr->buttonPtr.data(), &QAbstractButton::toggled, this, &IconFliter::onToggle);
    }
}
