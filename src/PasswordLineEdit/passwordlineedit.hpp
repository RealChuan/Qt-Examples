#pragma once

#include <QLineEdit>

class PasswordLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit PasswordLineEdit(QWidget *parent = nullptr);
    ~PasswordLineEdit() override;

private slots:
    void onShowPassword(bool state);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUI();
    void buildConnect();

    class PasswordLineEditPrivate;
    QScopedPointer<PasswordLineEditPrivate> d_ptr;
};
