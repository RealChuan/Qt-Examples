#include "mainwindow.hpp"
#include "passwordlineedit.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建主窗口部件
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    auto *mainLayout = new QVBoxLayout(centralWidget);

    // 创建多个密码输入框演示不同功能
    auto createPasswordField = [this](const QString &labelText,
                                      const QString &placeholder = "") -> auto * {
        auto *group = new QGroupBox(labelText, this);
        auto *layout = new QVBoxLayout(group);

        auto *passwordEdit = new PasswordLineEdit(this);
        if (!placeholder.isEmpty()) {
            passwordEdit->setPlaceholderText(placeholder);
        }

        layout->addWidget(passwordEdit);
        return group;
    };

    // 基本密码输入框
    mainLayout->addWidget(createPasswordField("Basic Password Field", "Enter your password here"));

    // 带自定义提示的密码框
    auto *customPlaceholderGroup = new QGroupBox("Password Field with Custom Placeholder", this);
    auto *customLayout = new QVBoxLayout(customPlaceholderGroup);
    auto *customPasswordEdit = new PasswordLineEdit(this);
    customPasswordEdit->setPlaceholderText("Minimum 8 characters with special symbols");
    customLayout->addWidget(customPasswordEdit);
    mainLayout->addWidget(customPlaceholderGroup);

    // 禁用CapsLock警告的密码框
    auto *noCapsLockGroup = new QGroupBox("Password Field without CapsLock Warning", this);
    auto *noCapsLayout = new QVBoxLayout(noCapsLockGroup);
    auto *noCapsPasswordEdit = new PasswordLineEdit(this);
    noCapsPasswordEdit->setCapsLockWarningEnabled(false);
    noCapsPasswordEdit->setPlaceholderText("CapsLock warning disabled for this field");
    noCapsLayout->addWidget(noCapsPasswordEdit);
    mainLayout->addWidget(noCapsLockGroup);

    // 控制面板
    auto *controlGroup = new QGroupBox("Controls", this);
    auto *controlLayout = new QGridLayout(controlGroup);

    // CapsLock警告开关
    auto *capsLockCheckbox = new QCheckBox("Enable CapsLock Warning", this);
    capsLockCheckbox->setChecked(true);
    controlLayout->addWidget(capsLockCheckbox, 0, 0);

    // 显示所有密码按钮
    auto *showAllButton = new QPushButton("Show All Passwords", this);
    controlLayout->addWidget(showAllButton, 0, 1);

    // 隐藏所有密码按钮
    auto *hideAllButton = new QPushButton("Hide All Passwords", this);
    controlLayout->addWidget(hideAllButton, 0, 2);

    // 清除所有密码按钮
    auto *clearAllButton = new QPushButton("Clear All Passwords", this);
    controlLayout->addWidget(clearAllButton, 1, 0);

    // 验证按钮
    auto *validateButton = new QPushButton("Validate Passwords", this);
    controlLayout->addWidget(validateButton, 1, 1);

    // 工具提示时长设置
    auto *tooltipDurationSlider = new QSlider(Qt::Horizontal, this);
    tooltipDurationSlider->setRange(1000, 10000);
    tooltipDurationSlider->setValue(3000);
    auto *durationLabel = new QLabel("Tooltip Duration: 3000ms", this);
    controlLayout->addWidget(new QLabel("Tooltip Duration:", this), 2, 0);
    controlLayout->addWidget(tooltipDurationSlider, 2, 1);
    controlLayout->addWidget(durationLabel, 2, 2);

    mainLayout->addWidget(controlGroup);

    // 状态显示
    auto *statusGroup = new QGroupBox("Status", this);
    auto *statusLayout = new QVBoxLayout(statusGroup);
    auto *statusLabel = new QLabel("Ready", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLayout->addWidget(statusLabel);
    mainLayout->addWidget(statusGroup);

    // 收集所有密码输入框
    QList<PasswordLineEdit *> passwordEdits;
    passwordEdits << customPasswordEdit << noCapsPasswordEdit;

    // 动态查找所有PasswordLineEdit实例
    auto findAllPasswordEdits = [this]() {
        QList<PasswordLineEdit *> edits;
        for (auto *child : findChildren<PasswordLineEdit *>()) {
            edits.append(child);
        }
        return edits;
    };

    // 信号连接

    // CapsLock警告开关
    connect(capsLockCheckbox, &QCheckBox::toggled, this, [findAllPasswordEdits](bool enabled) {
        for (auto *edit : findAllPasswordEdits()) {
            edit->setCapsLockWarningEnabled(enabled);
        }
    });

    // 显示所有密码
    connect(showAllButton, &QPushButton::clicked, this, [findAllPasswordEdits]() {
        for (auto *edit : findAllPasswordEdits()) {
            edit->setEchoMode(QLineEdit::Normal);
            // 由于原代码没有提供直接设置显示状态的方法，我们通过模拟按钮点击来实现
            // 在实际使用中，如果PasswordLineEdit提供了togglePasswordVisibility()方法，应该使用它
        }
    });

    // 隐藏所有密码
    connect(hideAllButton, &QPushButton::clicked, this, [findAllPasswordEdits]() {
        for (auto *edit : findAllPasswordEdits()) {
            edit->setEchoMode(QLineEdit::Password);
        }
    });

    // 清除所有密码
    connect(clearAllButton, &QPushButton::clicked, this, [findAllPasswordEdits]() {
        for (auto *edit : findAllPasswordEdits()) {
            edit->clear();
        }
    });

    // 验证密码
    connect(validateButton, &QPushButton::clicked, this, [findAllPasswordEdits, statusLabel]() {
        bool allValid = true;
        QStringList messages;

        for (auto *edit : findAllPasswordEdits()) {
            if (!edit->text().isEmpty()) {
                if (edit->text().length() < 6) {
                    messages.append(QString("'%1' is too short (min 6 characters)")
                                        .arg(edit->placeholderText().left(20)));
                    allValid = false;
                }
            }
        }

        if (allValid && messages.isEmpty()) {
            statusLabel->setText("✓ All passwords are valid");
        } else if (messages.isEmpty()) {
            statusLabel->setText("ℹ No passwords to validate");
        } else {
            statusLabel->setText("✗ " + messages.join("; "));
        }
    });

    // 工具提示时长设置
    connect(tooltipDurationSlider,
            &QSlider::valueChanged,
            this,
            [durationLabel, findAllPasswordEdits](int value) {
                durationLabel->setText(QString("Tooltip Duration: %1ms").arg(value));
                for (auto *edit : findAllPasswordEdits()) {
                    edit->setToolTipDuration(value);
                }
            });

    // 密码输入框文本变化连接
    auto updateStatusOnTextChange = [statusLabel]() {
        statusLabel->setText("Password field updated");
    };

    for (auto *edit : findAllPasswordEdits()) {
        connect(edit, &QLineEdit::textChanged, this, updateStatusOnTextChange);
    }

    // 窗口设置
    setWindowTitle("PasswordLineEdit Demonstration");
    resize(600, 450);
}

MainWindow::~MainWindow() {}
