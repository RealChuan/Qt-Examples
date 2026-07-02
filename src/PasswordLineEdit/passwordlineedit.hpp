#pragma once

#include <QLineEdit>

#include <memory>

class PasswordLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(bool passwordVisible READ passwordVisible WRITE setPasswordVisible NOTIFY
                   passwordVisibleChanged)
    Q_PROPERTY(bool capsLockWarningEnabled READ capsLockWarningEnabled WRITE
                   setCapsLockWarningEnabled NOTIFY capsLockWarningEnabledChanged)
    Q_PROPERTY(int toolTipDuration READ toolTipDuration WRITE setToolTipDuration NOTIFY
                   toolTipDurationChanged)

public:
    explicit PasswordLineEdit(QWidget *parent = nullptr);
    ~PasswordLineEdit() override;

    [[nodiscard]] auto passwordVisible() const -> bool;
    void setPasswordVisible(bool visible);

    [[nodiscard]] auto capsLockWarningEnabled() const -> bool;
    void setCapsLockWarningEnabled(bool enabled);

    [[nodiscard]] auto toolTipDuration() const -> int;
    void setToolTipDuration(int milliseconds);

    void setToggleIcons(const QIcon &visibleIcon, const QIcon &hiddenIcon);

public slots:
    void togglePasswordVisibility();

signals:
    void passwordVisibleChanged(bool visible);
    void capsLockWarningEnabledChanged(bool enabled);
    void toolTipDurationChanged(int milliseconds);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private slots:
    void onTogglePassword(bool checked);
    void hideCapsLockToolTip();

private:
    void setupUI();
    void setupIcons();
    void setupConnections();
    void showCapsLockWarning();

    class PasswordLineEditPrivate;
    std::unique_ptr<PasswordLineEditPrivate> d_ptr;
};
