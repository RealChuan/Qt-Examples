#include "mainwindow.hpp"
#include "passwordlineedit.hpp"

#include <QtWidgets>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // === 创建密码输入控件 ===
    auto *basicPasswordEdit = new PasswordLineEdit(this);
    basicPasswordEdit->setPlaceholderText(tr("Enter your password here"));

    auto *customPasswordEdit = new PasswordLineEdit(this);
    customPasswordEdit->setPlaceholderText(tr("Minimum 8 characters with special symbols"));

    // === 创建控件 ===
    auto *capsLockCheckbox = new QCheckBox(tr("Enable CapsLock Warning"), this);
    capsLockCheckbox->setChecked(true);

    auto *showAllButton = new QPushButton(tr("Show All"), this);
    auto *hideAllButton = new QPushButton(tr("Hide All"), this);
    auto *clearAllButton = new QPushButton(tr("Clear All"), this);
    auto *validateButton = new QPushButton(tr("Validate"), this);
    auto *resetButton = new QPushButton(tr("Reset"), this);

    // === 状态显示 ===
    auto *statusLabel = new QLabel(tr("Ready"), this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    auto *infoLabel = new QLabel(this);
    infoLabel->setAlignment(Qt::AlignCenter);

    // === 布局设置 ===
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(mainWidget);

    // 左侧：密码输入演示区域
    auto *displayLayout = new QVBoxLayout();

    auto *basicGroup = new QGroupBox(tr("Basic Password Field"), this);
    auto *basicLayout = new QVBoxLayout(basicGroup);
    basicLayout->addWidget(basicPasswordEdit);
    displayLayout->addWidget(basicGroup);

    auto *customGroup = new QGroupBox(tr("Password Field with Custom Placeholder"), this);
    auto *customLayout = new QVBoxLayout(customGroup);
    customLayout->addWidget(customPasswordEdit);
    displayLayout->addWidget(customGroup);
    displayLayout->addWidget(infoLabel);

    // 右侧：控制面板
    auto *controlPanel = new QWidget(this);
    auto *controlLayout = new QVBoxLayout(controlPanel);

    // 显示设置组
    auto *displaySettingsGroup = new QGroupBox(tr("Display Settings"), this);
    auto *displaySettingsLayout = new QVBoxLayout(displaySettingsGroup);
    displaySettingsLayout->addWidget(capsLockCheckbox);
    controlLayout->addWidget(displaySettingsGroup);

    // 操作按钮组
    auto *actionsGroup = new QGroupBox(tr("Actions"), this);
    auto *actionsLayout = new QGridLayout(actionsGroup);
    actionsLayout->addWidget(showAllButton, 0, 0);
    actionsLayout->addWidget(hideAllButton, 0, 1);
    actionsLayout->addWidget(clearAllButton, 1, 0);
    actionsLayout->addWidget(validateButton, 1, 1);
    actionsLayout->addWidget(resetButton, 2, 0, 1, 2);
    controlLayout->addWidget(actionsGroup);

    // 状态显示
    controlLayout->addWidget(statusLabel);

    // 主布局
    mainLayout->addLayout(displayLayout, 2);
    mainLayout->addWidget(controlPanel, 1);

    setCentralWidget(mainWidget);
    setWindowTitle(tr("Password Line Edit Example"));

    // === 收集所有密码输入框 ===
    QList<PasswordLineEdit *> passwordEdits = {basicPasswordEdit, customPasswordEdit};

    // === 更新信息标签 ===
    auto updateInfoLabel = [infoLabel, basicPasswordEdit, customPasswordEdit]() {
        infoLabel->setText(
            tr("Basic: %1 | Length: %2\nCustom: %3 | Length: %4")
                .arg(basicPasswordEdit->passwordVisible() ? tr("Visible") : tr("Hidden"))
                .arg(basicPasswordEdit->text().length())
                .arg(customPasswordEdit->passwordVisible() ? tr("Visible") : tr("Hidden"))
                .arg(customPasswordEdit->text().length()));
    };

    // === 信号连接 ===

    // CapsLock 警告开关
    connect(capsLockCheckbox, &QCheckBox::toggled, this, [passwordEdits](bool enabled) {
        for (auto *edit : passwordEdits) {
            edit->setCapsLockWarningEnabled(enabled);
        }
    });

    // 显示所有密码
    connect(showAllButton, &QPushButton::clicked, this, [passwordEdits]() {
        for (auto *edit : passwordEdits) {
            edit->setPasswordVisible(true);
        }
    });

    // 隐藏所有密码
    connect(hideAllButton, &QPushButton::clicked, this, [passwordEdits]() {
        for (auto *edit : passwordEdits) {
            edit->setPasswordVisible(false);
        }
    });

    // 清除所有密码
    connect(clearAllButton, &QPushButton::clicked, this, [passwordEdits]() {
        for (auto *edit : passwordEdits) {
            edit->clear();
        }
    });

    // 验证密码
    connect(validateButton, &QPushButton::clicked, this, [passwordEdits, statusLabel]() {
        bool allValid = true;
        QStringList messages;

        for (auto *edit : passwordEdits) {
            if (!edit->text().isEmpty() && edit->text().length() < 6) {
                messages.append(tr("'%1' is too short (min 6 characters)")
                                    .arg(edit->placeholderText().left(20)));
                allValid = false;
            }
        }

        if (allValid && messages.isEmpty()) {
            statusLabel->setText(u"✓ "_s + tr("All passwords are valid"));
        } else if (messages.isEmpty()) {
            statusLabel->setText(u"ℹ "_s + tr("No passwords to validate"));
        } else {
            statusLabel->setText(u"✗ "_s + messages.join(u"; "_s));
        }
    });

    // 重置
    connect(
        resetButton, &QPushButton::clicked, this, [passwordEdits, capsLockCheckbox, statusLabel]() {
            for (auto *edit : passwordEdits) {
                edit->clear();
                edit->setPasswordVisible(false);
                edit->setCapsLockWarningEnabled(true);
            }
            capsLockCheckbox->setChecked(true);
            statusLabel->setText(tr("Reset complete"));
        });

    // 密码可见性变化 → 更新信息标签
    for (auto *edit : passwordEdits) {
        connect(edit, &PasswordLineEdit::passwordVisibleChanged, this, updateInfoLabel);
        connect(edit, &QLineEdit::textChanged, this, updateInfoLabel);
    }

    // 初始化
    updateInfoLabel();
    resize(600, 240);
}
