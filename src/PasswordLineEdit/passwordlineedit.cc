#include "passwordlineedit.hpp"

#include <QtWidgets>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {

auto checkCapsLockState() -> bool
{
#ifdef Q_OS_WIN
    return (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
#else
    qWarning() << "CapsLock detection is not supported on this platform";
    return false;
#endif
}

} // namespace

class PasswordLineEdit::PasswordLineEditPrivate
{
public:
    explicit PasswordLineEditPrivate(PasswordLineEdit *q) : q_ptr(q)
    {
        toggleButton = new QToolButton(q_ptr);
        toggleButton->setIconSize({20, 20});
        toggleButton->setCursor(Qt::PointingHandCursor);
        toggleButton->setCheckable(true);
        toggleButton->setToolTip(q_ptr->tr("Show password"));

        capsLockToolTipLabel = new QLabel(q_ptr->window());
        capsLockToolTipLabel->setWindowFlags(Qt::ToolTip);
        capsLockToolTipLabel->setStyleSheet(
            "QLabel{background: #FFF3CD; border: 1px solid #FFEaa8; padding: 8px; "
            "color: #856404; font-size: 12px; border-radius: 4px;}");
        capsLockToolTipLabel->hide();

        capsLockHideTimer = new QTimer(q_ptr);
        capsLockHideTimer->setSingleShot(true);
    }

    PasswordLineEdit *q_ptr;
    QToolButton *toggleButton;
    QIcon visibleIcon{};
    QIcon hiddenIcon{};
    QLabel *capsLockToolTipLabel;
    QTimer *capsLockHideTimer;
    bool isCapsLockWarningEnabled = true;
};

PasswordLineEdit::PasswordLineEdit(QWidget *parent)
    : QLineEdit(parent), d_ptr(std::make_unique<PasswordLineEditPrivate>(this))
{
    setupIcons();
    setupUI();
    setupConnections();

    const QRegularExpression regExp(R"([\x20-\x7E]+)");
    setValidator(new QRegularExpressionValidator(regExp, this));

    setAttribute(Qt::WA_InputMethodEnabled, false);
    setEchoMode(QLineEdit::Password);

    installEventFilter(this);
}

PasswordLineEdit::~PasswordLineEdit() = default;

auto PasswordLineEdit::passwordVisible() const -> bool
{ return echoMode() == QLineEdit::Normal; }

void PasswordLineEdit::setPasswordVisible(bool visible)
{
    if (visible == passwordVisible()) {
        return;
    }
    setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    d_ptr->toggleButton->blockSignals(true);
    d_ptr->toggleButton->setChecked(visible);
    d_ptr->toggleButton->setIcon(visible ? d_ptr->visibleIcon : d_ptr->hiddenIcon);
    d_ptr->toggleButton->setToolTip(visible ? tr("Hide password") : tr("Show password"));
    d_ptr->toggleButton->blockSignals(false);
    emit passwordVisibleChanged(visible);
}

auto PasswordLineEdit::capsLockWarningEnabled() const -> bool
{ return d_ptr->isCapsLockWarningEnabled; }

void PasswordLineEdit::setCapsLockWarningEnabled(bool enabled)
{
    if (d_ptr->isCapsLockWarningEnabled == enabled) {
        return;
    }
    d_ptr->isCapsLockWarningEnabled = enabled;
    if (!enabled) {
        d_ptr->capsLockToolTipLabel->hide();
    }
    emit capsLockWarningEnabledChanged(enabled);
}

auto PasswordLineEdit::toolTipDuration() const -> int
{ return d_ptr->capsLockHideTimer->interval(); }

void PasswordLineEdit::setToolTipDuration(int milliseconds)
{
    if (d_ptr->capsLockHideTimer->interval() == milliseconds) {
        return;
    }
    d_ptr->capsLockHideTimer->setInterval(milliseconds);
    emit toolTipDurationChanged(milliseconds);
}

void PasswordLineEdit::setToggleIcons(const QIcon &visibleIcon, const QIcon &hiddenIcon)
{
    d_ptr->visibleIcon = visibleIcon;
    d_ptr->hiddenIcon = hiddenIcon;
    d_ptr->toggleButton->setIcon(passwordVisible() ? visibleIcon : hiddenIcon);
}

void PasswordLineEdit::togglePasswordVisibility()
{ setPasswordVisible(!passwordVisible()); }

bool PasswordLineEdit::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && d_ptr->isCapsLockWarningEnabled && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_CapsLock) {
            QTimer::singleShot(0, this, &PasswordLineEdit::showCapsLockWarning);
        }
    }
    return QLineEdit::eventFilter(watched, event);
}

void PasswordLineEdit::focusInEvent(QFocusEvent *event)
{
    QLineEdit::focusInEvent(event);
    if (d_ptr->isCapsLockWarningEnabled) {
        showCapsLockWarning();
    }
}

void PasswordLineEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
    d_ptr->capsLockToolTipLabel->hide();
}

void PasswordLineEdit::onTogglePassword(bool checked)
{ setPasswordVisible(checked); }

void PasswordLineEdit::hideCapsLockToolTip()
{ d_ptr->capsLockToolTipLabel->hide(); }

void PasswordLineEdit::setupUI()
{
    setPlaceholderText(tr("Enter password"));
    setMinimumSize(200, 35);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(d_ptr->toggleButton);
}

void PasswordLineEdit::setupIcons()
{
    if (!d_ptr->visibleIcon.isNull() && !d_ptr->hiddenIcon.isNull()) {
        return;
    }

    auto *style = QApplication::style();
    d_ptr->visibleIcon = style->standardIcon(QStyle::SP_DialogOpenButton);
    d_ptr->hiddenIcon = style->standardIcon(QStyle::SP_DialogApplyButton);
    d_ptr->toggleButton->setIcon(d_ptr->hiddenIcon);
}

void PasswordLineEdit::setupConnections()
{
    connect(d_ptr->toggleButton, &QToolButton::toggled, this, &PasswordLineEdit::onTogglePassword);
    connect(
        d_ptr->capsLockHideTimer, &QTimer::timeout, this, &PasswordLineEdit::hideCapsLockToolTip);
}

void PasswordLineEdit::showCapsLockWarning()
{
    if (!checkCapsLockState()) {
        return;
    }

    d_ptr->capsLockToolTipLabel->setText(tr("Caps Lock is ON"));
    d_ptr->capsLockToolTipLabel->adjustSize();

    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    int x = globalPos.x();
    int y = globalPos.y() - d_ptr->capsLockToolTipLabel->height() - 5;

    auto *handle = window()->windowHandle();
    if (!handle) {
        return;
    }
    QScreen *screen = handle->screen();
    if (!screen) {
        return;
    }

    const QRect screenGeometry = screen->availableGeometry();
    if (y < screenGeometry.top()) {
        y = globalPos.y() + height() + 5;
    }
    if (x + d_ptr->capsLockToolTipLabel->width() > screenGeometry.right()) {
        x = screenGeometry.right() - d_ptr->capsLockToolTipLabel->width();
    }

    d_ptr->capsLockToolTipLabel->move(x, y);
    d_ptr->capsLockToolTipLabel->show();
    d_ptr->capsLockHideTimer->start(toolTipDuration() > 0 ? toolTipDuration() : 3000);
}
