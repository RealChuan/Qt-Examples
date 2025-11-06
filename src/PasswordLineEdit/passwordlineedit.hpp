#pragma once

#include <QLineEdit>

class PasswordLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(
        bool capsLockWarningEnabled READ capsLockWarningEnabled WRITE setCapsLockWarningEnabled)

public:
    explicit PasswordLineEdit(QWidget *parent = nullptr);
    ~PasswordLineEdit() override;

    bool capsLockWarningEnabled() const;
    void setCapsLockWarningEnabled(bool enabled);

    void setToggleIcons(const QIcon &visibleIcon, const QIcon &hiddenIcon);
    void setToolTipDuration(int milliseconds);

public slots:
    void togglePasswordVisibility();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private slots:
    void onShowPassword(bool visible);
    void hideToolTip();

private:
    void setupUI();
    void setupIcons();
    void setupConnections();
    void showCapsLockWarning();

    class PasswordLineEditPrivate;
    QScopedPointer<PasswordLineEditPrivate> d_ptr;
};
