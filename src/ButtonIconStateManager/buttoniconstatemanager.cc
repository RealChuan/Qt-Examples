#include "buttoniconstatemanager.hpp"

#include <QEnterEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QPointer>

namespace {

QIcon createIconFromPaths(const QStringList &paths)
{
    QIcon icon;
    for (const QString &path : paths) {
        if (!path.isEmpty()) {
            icon.addFile(path);
        }
    }
    return icon;
}

} // namespace

class ButtonIconStateManager::ButtonIconStateManagerPrivate
{
public:
    explicit ButtonIconStateManagerPrivate(ButtonIconStateManager *q)
        : q_ptr(q)
    {}

    ButtonIconStateManager *q_ptr;

    QPointer<QAbstractButton> buttonPtr;

    QIcon normalIcon;
    QIcon hoverIcon;
    QIcon pressedIcon;
    QIcon checkedIcon;

    bool isMousePressed = false;
    bool isMouseHover = false;
};

ButtonIconStateManager::ButtonIconStateManager(QAbstractButton *parent)
    : QObject(parent)
    , d_ptr(new ButtonIconStateManagerPrivate(this))
{
    d_ptr->buttonPtr = parent;
    if (d_ptr->buttonPtr) {
        d_ptr->buttonPtr->installEventFilter(this);
        setupConnections();

        // 设置初始图标
        if (!d_ptr->normalIcon.isNull()) {
            d_ptr->buttonPtr->setIcon(d_ptr->normalIcon);
        }
    }
}

ButtonIconStateManager::~ButtonIconStateManager()
{
    if (d_ptr->buttonPtr) {
        d_ptr->buttonPtr->removeEventFilter(this);
    }
}

void ButtonIconStateManager::setNormalIcon(const QIcon &icon)
{
    if (!icon.isNull()) {
        d_ptr->normalIcon = icon;
        if (d_ptr->buttonPtr && !d_ptr->isMouseHover && !d_ptr->isMousePressed) {
            d_ptr->buttonPtr->setIcon(d_ptr->normalIcon);
        }
    }
}

void ButtonIconStateManager::setNormalIcon(const QStringList &iconPaths)
{
    setNormalIcon(createIconFromPaths(iconPaths));
}

void ButtonIconStateManager::setHoverIcon(const QIcon &icon)
{
    if (!icon.isNull()) {
        d_ptr->hoverIcon = icon;
    }
}

void ButtonIconStateManager::setHoverIcon(const QStringList &iconPaths)
{
    setHoverIcon(createIconFromPaths(iconPaths));
}

void ButtonIconStateManager::setPressedIcon(const QIcon &icon)
{
    if (!icon.isNull()) {
        d_ptr->pressedIcon = icon;
    }
}

void ButtonIconStateManager::setPressedIcon(const QStringList &iconPaths)
{
    setPressedIcon(createIconFromPaths(iconPaths));
}

void ButtonIconStateManager::setCheckedIcon(const QIcon &icon)
{
    if (!icon.isNull()) {
        d_ptr->checkedIcon = icon;
        updateIconBasedOnState();
    }
}

void ButtonIconStateManager::setCheckedIcon(const QStringList &iconPaths)
{
    setCheckedIcon(createIconFromPaths(iconPaths));
}

void ButtonIconStateManager::resetToNormal()
{
    if (d_ptr->buttonPtr) {
        d_ptr->buttonPtr->setIcon(d_ptr->normalIcon);
    }
}

void ButtonIconStateManager::onButtonToggled(bool checked)
{
    Q_UNUSED(checked)
    updateIconBasedOnState();
}

bool ButtonIconStateManager::eventFilter(QObject *watched, QEvent *event)
{
    if (!d_ptr->buttonPtr || watched != d_ptr->buttonPtr) {
        return QObject::eventFilter(watched, event);
    }

    switch (event->type()) {
    case QEvent::Enter:
        d_ptr->isMouseHover = true;
        updateIconBasedOnState();
        break;
    case QEvent::Leave:
        d_ptr->isMouseHover = false;
        updateIconBasedOnState();
        break;
    case QEvent::MouseButtonPress:
        if (static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton) {
            d_ptr->isMousePressed = true;
            updateIconBasedOnState();
        }
        break;
    case QEvent::MouseButtonRelease:
        if (static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton) {
            d_ptr->isMousePressed = false;
            updateIconBasedOnState();
        }
        break;
    case QEvent::EnabledChange: updateIconBasedOnState(); break;
    default: break;
    }

    return QObject::eventFilter(watched, event);
}

void ButtonIconStateManager::setupConnections()
{
    if (d_ptr->buttonPtr) {
        connect(d_ptr->buttonPtr,
                &QAbstractButton::toggled,
                this,
                &ButtonIconStateManager::onButtonToggled,
                Qt::UniqueConnection);
    }
}

void ButtonIconStateManager::updateIconBasedOnState()
{
    if (!d_ptr->buttonPtr || !d_ptr->buttonPtr->isEnabled()) {
        return;
    }

    QIcon targetIcon;

    if (d_ptr->isMousePressed && !d_ptr->pressedIcon.isNull()) {
        // 鼠标按下状态
        targetIcon = d_ptr->pressedIcon;
    } else if (d_ptr->isMouseHover && !d_ptr->hoverIcon.isNull()) {
        // 鼠标悬停状态
        targetIcon = d_ptr->hoverIcon;
    } else if (d_ptr->buttonPtr->isChecked() && !d_ptr->checkedIcon.isNull()) {
        // 选中状态
        targetIcon = d_ptr->checkedIcon;
    } else {
        // 正常状态
        targetIcon = d_ptr->normalIcon;
    }

    if (!targetIcon.isNull()) {
        d_ptr->buttonPtr->setIcon(targetIcon);
    }
}
