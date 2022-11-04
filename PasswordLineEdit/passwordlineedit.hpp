#ifndef PASSWORDLINEEDIT_HPP
#define PASSWORDLINEEDIT_HPP

#include <QLineEdit>

class PasswordLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit PasswordLineEdit(QWidget *parent = nullptr);

private slots:
    void onShowPassword(bool);

private:
    QToolButton *m_toolButton;
    QIcon m_hidden_icon;
    QIcon m_view_icon;
};

#endif // PASSWORDLINEEDIT_HPP
