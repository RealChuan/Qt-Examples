#pragma once

#include <QAbstractButton>

#include <memory>

class ButtonIconStateManager : public QObject
{
    Q_OBJECT

public:
    explicit ButtonIconStateManager(QAbstractButton *parent = nullptr);
    ~ButtonIconStateManager() override;

    // 设置正常状态图标
    void setNormalIcon(const QIcon &icon);
    void setNormalIcon(const QStringList &iconPaths);

    // 设置悬停状态图标
    void setHoverIcon(const QIcon &icon);
    void setHoverIcon(const QStringList &iconPaths);

    // 设置按下状态图标
    void setPressedIcon(const QIcon &icon);
    void setPressedIcon(const QStringList &iconPaths);

    // 设置选中状态图标（可选）
    void setCheckedIcon(const QIcon &icon);
    void setCheckedIcon(const QStringList &iconPaths);

    // 重置为正常状态
    void resetToNormal();

private slots:
    void onButtonToggled([[maybe_unused]] bool checked);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupConnections();
    void updateIconBasedOnState();

    class ButtonIconStateManagerPrivate;
    std::unique_ptr<ButtonIconStateManagerPrivate> d_ptr;
};
