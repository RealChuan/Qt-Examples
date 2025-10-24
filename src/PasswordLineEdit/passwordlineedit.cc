#include "passwordlineedit.hpp"

#include <QtWidgets>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

bool isCapsLockOn()
{
#ifdef Q_OS_WIN
    return (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
#else
    qWarning() << "Unsupported platform";
    return false;
#endif
}

class PasswordLineEdit::PasswordLineEditPrivate
{
public:
    explicit PasswordLineEditPrivate(PasswordLineEdit *q)
        : q_ptr(q)
    {
        toolButton = new QToolButton(q_ptr);
        toolButton->setIconSize({25, 25});
        toolButton->setCursor(Qt::PointingHandCursor);
        toolButton->setCheckable(true);

        hiddenIcon.addFile("://resources/hidden_password.png");
        viewIcon.addFile("://resources/view_password.png");

        labelPtr.reset(new QLabel);
        labelPtr->setWindowFlags(labelPtr->windowFlags() | Qt::ToolTip);
        labelPtr->setStyleSheet("QLabel{background: #909090; padding:5px; color: #E60000;"
                                "font-size:12px; font-weight:bold;}");
        labelPtr->hide();
        timer = new QTimer(q_ptr);
        timer->setInterval(3000);
    }

    ~PasswordLineEditPrivate() {}

    void showLabel(const QString &text)
    {
        labelPtr->setText(text);
        labelPtr->show();
        auto point = q_ptr->mapToGlobal(QPoint(10, -labelPtr->height() / 2));
        labelPtr->move(point);
        timer->start();
    }

    PasswordLineEdit *q_ptr;

    QToolButton *toolButton;
    QIcon hiddenIcon;
    QIcon viewIcon;

    QScopedPointer<QLabel> labelPtr;
    QTimer *timer;
};

PasswordLineEdit::PasswordLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , d_ptr(new PasswordLineEditPrivate(this))
{
    setupUI();
    buildConnect();
#ifdef Q_OS_WIN
    installEventFilter(this);
#endif
    onShowPassword(false);
}

PasswordLineEdit::~PasswordLineEdit() {}

void PasswordLineEdit::onShowPassword(bool state)
{
    if (state) {
        setEchoMode(Normal);
        d_ptr->toolButton->setIcon(d_ptr->hiddenIcon);
    } else {
        setEchoMode(Password);
        d_ptr->toolButton->setIcon(d_ptr->viewIcon);
    }
}

bool PasswordLineEdit::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        switch (event->type()) {
        case QEvent::KeyRelease: {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_CapsLock) {
                d_ptr->showLabel(isCapsLockOn() ? tr("Caps Lock On") : tr("Caps Lock Off"));
            }
        } break;
        case QEvent::FocusIn:
            if (isCapsLockOn()) {
                d_ptr->showLabel(tr("Caps Lock On"));
            }
            break;
        default: break;
        }
    }
    return QLineEdit::eventFilter(watched, event);
}

void PasswordLineEdit::setupUI()
{
    setStyleSheet("QLineEdit{border:none; border-bottom: 1px solid #DDDDDD;"
                  "color: #222222; padding-left:10px; font-size:14px; }"
                  "QToolButton{border: none;}");
    setPlaceholderText(tr("Please enter the password."));
    setMinimumSize(200, 30);

    QRegularExpression regExp("[a-zA-Z0-9!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~]+");
    setValidator(new QRegularExpressionValidator(regExp, this));
    setAttribute(Qt::WA_InputMethodEnabled, false);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(1, 1, 10, 1);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(d_ptr->toolButton);
}

void PasswordLineEdit::buildConnect()
{
    connect(d_ptr->toolButton, &QToolButton::clicked, this, &PasswordLineEdit::onShowPassword);
    connect(d_ptr->timer, &QTimer::timeout, d_ptr->labelPtr.data(), &QLabel::hide);
}
