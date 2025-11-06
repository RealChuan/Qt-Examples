#include "passwordlineedit.hpp"
#include <QtWidgets>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {

bool checkCapsLockState()
{
#ifdef Q_OS_WIN
    return (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
#else
    qWarning() << "Unsupported platform";
    return false;
#endif
}

} // namespace

class PasswordLineEdit::PasswordLineEditPrivate
{
public:
    explicit PasswordLineEditPrivate(PasswordLineEdit *q)
        : q_ptr(q)
    {
        toolButton = new QToolButton(q_ptr);
        toolButton->setIconSize({20, 20});
        toolButton->setCursor(Qt::PointingHandCursor);
        toolButton->setCheckable(true);
        toolButton->setToolTip(q_ptr->tr("Show/hide password"));

        toolTipLabel = new QLabel(q_ptr->window());
        toolTipLabel->setWindowFlags(Qt::ToolTip);
        toolTipLabel->setStyleSheet("QLabel{"
                                    "background: #FFF3CD;"
                                    "border: 1px solid #FFEaa8;"
                                    "padding: 8px;"
                                    "color: #856404;"
                                    "font-size: 12px;"
                                    "border-radius: 4px;"
                                    "}");
        toolTipLabel->hide();

        toolTipTimer = new QTimer(q_ptr);
        toolTipTimer->setSingleShot(true);
    }

    PasswordLineEdit *q_ptr;
    QToolButton *toolButton;
    QIcon visibleIcon;
    QIcon hiddenIcon;
    QLabel *toolTipLabel;
    QTimer *toolTipTimer;
    bool capsLockWarning = true;
};

PasswordLineEdit::PasswordLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , d_ptr(new PasswordLineEditPrivate(this))
{
    setupIcons();
    setupUI();
    setupConnections();

    // 设置合理的密码验证
    QRegularExpression regExp(R"([\x20-\x7E]+)"); // 所有可打印ASCII字符
    setValidator(new QRegularExpressionValidator(regExp, this));

    setAttribute(Qt::WA_InputMethodEnabled, false);
    setEchoMode(QLineEdit::Password);

    installEventFilter(this);
}

PasswordLineEdit::~PasswordLineEdit() = default;

bool PasswordLineEdit::capsLockWarningEnabled() const
{
    return d_ptr->capsLockWarning;
}

void PasswordLineEdit::setCapsLockWarningEnabled(bool enabled)
{
    d_ptr->capsLockWarning = enabled;
}

void PasswordLineEdit::setToggleIcons(const QIcon &visibleIcon, const QIcon &hiddenIcon)
{
    d_ptr->visibleIcon = visibleIcon;
    d_ptr->hiddenIcon = hiddenIcon;
    d_ptr->toolButton->setIcon(hiddenIcon);
}

void PasswordLineEdit::setToolTipDuration(int milliseconds)
{
    d_ptr->toolTipTimer->setInterval(milliseconds);
}

void PasswordLineEdit::togglePasswordVisibility()
{
    d_ptr->toolButton->toggle();
}

bool PasswordLineEdit::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && d_ptr->capsLockWarning) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_CapsLock) {
                // 延迟检查，确保状态已更新
                QTimer::singleShot(0, this, &PasswordLineEdit::showCapsLockWarning);
            }
        }
    }
    return QLineEdit::eventFilter(watched, event);
}

void PasswordLineEdit::focusInEvent(QFocusEvent *event)
{
    QLineEdit::focusInEvent(event);
    if (d_ptr->capsLockWarning) {
        showCapsLockWarning();
    }
}

void PasswordLineEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
    d_ptr->toolTipLabel->hide();
}

void PasswordLineEdit::onShowPassword(bool visible)
{
    setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);

    d_ptr->toolButton->setIcon(visible ? d_ptr->visibleIcon : d_ptr->hiddenIcon);
    d_ptr->toolButton->setToolTip(visible ? tr("Hide password") : tr("Show password"));
}

void PasswordLineEdit::hideToolTip()
{
    d_ptr->toolTipLabel->hide();
}

void PasswordLineEdit::setupUI()
{
    setPlaceholderText(tr("Enter password"));
    setMinimumSize(200, 35);

    // 使用布局确保按钮正确对齐
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(d_ptr->toolButton);
}

void PasswordLineEdit::setupIcons()
{
    // 尝试加载自定义图标，失败则使用系统图标
    if (!d_ptr->visibleIcon.isNull() && !d_ptr->hiddenIcon.isNull()) {
        return; // 图标已设置
    }

    // 使用系统图标作为备选
    d_ptr->visibleIcon = QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView);
    d_ptr->hiddenIcon = QApplication::style()->standardIcon(QStyle::SP_FileDialogContentsView);
    d_ptr->toolButton->setIcon(d_ptr->hiddenIcon);
}

void PasswordLineEdit::setupConnections()
{
    connect(d_ptr->toolButton, &QToolButton::toggled, this, &PasswordLineEdit::onShowPassword);
    connect(d_ptr->toolTipTimer, &QTimer::timeout, this, &PasswordLineEdit::hideToolTip);
}

void PasswordLineEdit::showCapsLockWarning()
{
    if (!checkCapsLockState()) {
        return;
    }

    d_ptr->toolTipLabel->setText(tr("Caps Lock is ON"));
    d_ptr->toolTipLabel->adjustSize();

    // 计算提示位置
    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    int x = globalPos.x();
    int y = globalPos.y() - d_ptr->toolTipLabel->height() - 5;

    // 边界检查
    QScreen *screen = window()->windowHandle()->screen();
    QRect screenGeometry = screen->availableGeometry();
    if (y < screenGeometry.top()) {
        y = globalPos.y() + height() + 5;
    }
    if (x + d_ptr->toolTipLabel->width() > screenGeometry.right()) {
        x = screenGeometry.right() - d_ptr->toolTipLabel->width();
    }

    d_ptr->toolTipLabel->move(x, y);
    d_ptr->toolTipLabel->show();
    d_ptr->toolTipTimer->start(3000); // 3秒后自动隐藏
}
