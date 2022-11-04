#include "passwordlineedit.hpp"

#include <QtWidgets>

PasswordLineEdit::PasswordLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setAttribute(Qt::WA_InputMethodEnabled, false);
    setStyleSheet("QLineEdit{ border:none; border-bottom: 1px solid #DDDDDD; color: #222222; "
                  "padding-left:10px; font-size:14px; }"
                  "QToolButton{ border: none; }");
    setPlaceholderText(tr("Please enter the password."));
    setMinimumSize(200, 30);

    m_hidden_icon.addFile("://resources/hidden_password.png");
    m_view_icon.addFile("://resources/view_password.png");

    m_toolButton = new QToolButton(this);
    m_toolButton->setIconSize(QSize(25, 25));
    m_toolButton->setCursor(Qt::PointingHandCursor);
    m_toolButton->setCheckable(true);
    connect(m_toolButton, &QToolButton::clicked, this, &PasswordLineEdit::onShowPassword);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(1, 1, 10, 1);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(m_toolButton);

    onShowPassword(false);
}

void PasswordLineEdit::onShowPassword(bool state)
{
    if (state) {
        setEchoMode(Normal);
        m_toolButton->setIcon(m_hidden_icon);
    } else {
        setEchoMode(Password);
        m_toolButton->setIcon(m_view_icon);
    }
}
