#include "mainwindow.hpp"
#include "switchbutton.hpp"

#include <QtWidgets>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // 创建开关按钮控件
    auto *switchButton = new SwitchButton(this);
    switchButton->setFixedSize(80, 40);

    // 创建状态显示标签
    auto *statusLabel = new QLabel(tr("Unchecked"), this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    statusLabel->setMinimumHeight(30);

    // 创建状态控制
    auto *stateCheckbox = new QCheckBox(tr("Checked"), this);

    // 创建颜色选择控件
    auto *checkedColorButton = new QPushButton(this);
    auto *uncheckedColorButton = new QPushButton(this);
    auto *thumbColorButton = new QPushButton(this);
    auto *thumbBorderColorButton = new QPushButton(this);

    // 创建动画控制
    auto *animationDurationSlider = new QSlider(Qt::Horizontal, this);
    animationDurationSlider->setRange(0, 1000);
    animationDurationSlider->setValue(switchButton->animationDuration());
    auto *durationLabel
        = new QLabel(tr("Animation duration: %1ms").arg(switchButton->animationDuration()));

    // ========== 布局设置 ==========

    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(mainWidget);

    // 左侧：展示区域
    auto *displayLayout = new QVBoxLayout();
    displayLayout->addStretch();
    displayLayout->addWidget(switchButton, 0, Qt::AlignCenter);
    displayLayout->addSpacing(10);
    displayLayout->addWidget(statusLabel);
    displayLayout->addStretch();

    // 右侧：控制面板
    auto *controlPanel = new QWidget(this);
    auto *controlLayout = new QVBoxLayout(controlPanel);

    // 状态控制布局
    auto *stateGroup = new QGroupBox(tr("State"), this);
    auto *stateLayout = new QVBoxLayout(stateGroup);
    stateLayout->addWidget(stateCheckbox);

    // 颜色控制布局
    auto *colorGroup = new QGroupBox(tr("Color settings"), this);
    auto *colorLayout = new QGridLayout(colorGroup);

    colorLayout->addWidget(new QLabel(tr("Checked color:"), this), 0, 0);
    colorLayout->addWidget(checkedColorButton, 0, 1);
    colorLayout->addWidget(new QLabel(tr("Unchecked color:"), this), 1, 0);
    colorLayout->addWidget(uncheckedColorButton, 1, 1);
    colorLayout->addWidget(new QLabel(tr("Thumb color:"), this), 2, 0);
    colorLayout->addWidget(thumbColorButton, 2, 1);
    colorLayout->addWidget(new QLabel(tr("Thumb border:"), this), 3, 0);
    colorLayout->addWidget(thumbBorderColorButton, 3, 1);

    // 动画控制布局
    auto *animationGroup = new QGroupBox(tr("Animation settings"), this);
    auto *animationLayout = new QVBoxLayout(animationGroup);
    animationLayout->addWidget(durationLabel);
    animationLayout->addWidget(animationDurationSlider);

    // 组装控制面板
    controlLayout->addWidget(stateGroup);
    controlLayout->addWidget(colorGroup);
    controlLayout->addWidget(animationGroup);
    controlLayout->addStretch();

    // 主布局
    mainLayout->addLayout(displayLayout, 2);
    mainLayout->addWidget(controlPanel);

    setCentralWidget(mainWidget);
    resize(600, 350);
    setWindowTitle(tr("Switch Button Example"));

    // ========== 颜色按钮更新 ==========

    auto updateColorButton = [](QPushButton *button, const QColor &color) {
        auto colorName = color.name(QColor::HexArgb).toUpper();

        // 计算相对亮度（sRGB颜色空间）
        auto getRelativeLuminance = [](int r, int g, int b) {
            auto normalize = [](double x) {
                return x <= 0.03928 ? x / 12.92 : std::pow((x + 0.055) / 1.055, 2.4);
            };
            return 0.2126 * normalize(r / 255.0) + 0.7152 * normalize(g / 255.0)
                   + 0.0722 * normalize(b / 255.0);
        };

        double luminance = getRelativeLuminance(color.red(), color.green(), color.blue());
        QString textColor = luminance > 0.179 ? u"black"_s : u"white"_s;

        button->setStyleSheet(
            u"background-color: %1; color: %2; border: 1px solid gray; padding: 5px;"_s
                .arg(colorName)
                .arg(textColor));
        button->setText(colorName);
    };

    // 初始化颜色按钮
    updateColorButton(checkedColorButton, switchButton->checkedColor());
    updateColorButton(uncheckedColorButton, switchButton->uncheckedColor());
    updateColorButton(thumbColorButton, switchButton->thumbColor());
    updateColorButton(thumbBorderColorButton, switchButton->thumbBorderColor());

    // ========== 信号连接 ==========

    // 状态控制
    connect(stateCheckbox, &QCheckBox::toggled, this, [switchButton, stateCheckbox](bool checked) {
        stateCheckbox->blockSignals(true);
        switchButton->setChecked(checked);
        stateCheckbox->blockSignals(false);
    });

    // 颜色设置
    connect(checkedColorButton,
            &QPushButton::clicked,
            this,
            [this, switchButton, checkedColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(
                    switchButton->checkedColor(), this, tr("Select Checked Color"));
                if (color.isValid()) {
                    switchButton->setCheckedColor(color);
                    updateColorButton(checkedColorButton, color);
                }
            });

    connect(uncheckedColorButton,
            &QPushButton::clicked,
            this,
            [this, switchButton, uncheckedColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(
                    switchButton->uncheckedColor(), this, tr("Select Unchecked Color"));
                if (color.isValid()) {
                    switchButton->setUncheckedColor(color);
                    updateColorButton(uncheckedColorButton, color);
                }
            });

    connect(thumbColorButton,
            &QPushButton::clicked,
            this,
            [this, switchButton, thumbColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(
                    switchButton->thumbColor(), this, tr("Select Thumb Color"));
                if (color.isValid()) {
                    switchButton->setThumbColor(color);
                    updateColorButton(thumbColorButton, color);
                }
            });

    connect(thumbBorderColorButton,
            &QPushButton::clicked,
            this,
            [this, switchButton, thumbBorderColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(
                    switchButton->thumbBorderColor(), this, tr("Select Thumb Border Color"));
                if (color.isValid()) {
                    switchButton->setThumbBorderColor(color);
                    updateColorButton(thumbBorderColorButton, color);
                }
            });

    // 动画设置
    connect(animationDurationSlider,
            &QSlider::valueChanged,
            this,
            [switchButton, durationLabel](int value) {
                switchButton->setAnimationDuration(value);
                durationLabel->setText(tr("Animation duration: %1ms").arg(value));
            });

    // 开关按钮信号
    connect(switchButton, &SwitchButton::toggled, this, [stateCheckbox, statusLabel](bool checked) {
        // 更新状态复选框
        stateCheckbox->blockSignals(true);
        stateCheckbox->setChecked(checked);
        stateCheckbox->blockSignals(false);

        // 更新状态标签
        if (checked) {
            statusLabel->setText(tr("Checked"));
            statusLabel->setStyleSheet(u"color: green;"_s);
        } else {
            statusLabel->setText(tr("Unchecked"));
            statusLabel->setStyleSheet(QString{});
        }
    });

    connect(switchButton, &SwitchButton::animationStarted, this, [statusLabel](bool checked) {
        QString state = checked ? tr("Checked") : tr("Unchecked");
        statusLabel->setText(tr("Animating to: %1").arg(state));
        statusLabel->setStyleSheet(u"color: orange;"_s);
    });

    connect(switchButton, &SwitchButton::animationFinished, this, [statusLabel](bool checked) {
        QString state = checked ? tr("Checked") : tr("Unchecked");
        statusLabel->setText(tr("Finished: %1").arg(state));
        statusLabel->setStyleSheet(checked ? u"color: green;"_s : QString{});
    });
}
