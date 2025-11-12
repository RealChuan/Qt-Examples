#include "mainwindow.h"
#include "switchbutton.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建开关按钮控件
    auto *switchButton = new SwitchButton(this);

    // 创建状态控制
    auto *stateCheckbox = new QCheckBox(tr("Checked State"), this);
    stateCheckbox->setChecked(false);

    // 创建颜色选择控件
    auto *checkedColorButton = new QPushButton(this);
    auto *uncheckedColorButton = new QPushButton(this);
    auto *thumbColorButton = new QPushButton(this);
    auto *thumbBorderColorButton = new QPushButton(this);

    // 创建动画控制
    auto *animationDurationSlider = new QSlider(Qt::Horizontal, this);
    animationDurationSlider->setRange(50, 1000);
    animationDurationSlider->setValue(120);
    auto *durationLabel = new QLabel(tr("Animation duration: 120ms"), this);

    // 创建状态显示标签
    auto *statusLabel = new QLabel(tr("Status: Unchecked"), this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::Box);
    statusLabel->setMinimumHeight(30);

    // 布局设置
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    // 控制面板区域
    auto *controlPanel = new QWidget(this);
    auto *controlLayout = new QGridLayout(controlPanel);

    // 第一行：状态控制
    int row = 0;
    controlLayout->addWidget(new QLabel(tr("Button state:"), this), row, 0);
    controlLayout->addWidget(stateCheckbox, row, 1);

    // 第二行：颜色控制 - 选中颜色
    row++;
    controlLayout->addWidget(new QLabel(tr("Checked color:"), this), row, 0);
    controlLayout->addWidget(checkedColorButton, row, 1);

    // 第三行：颜色控制 - 未选中颜色
    row++;
    controlLayout->addWidget(new QLabel(tr("Unchecked color:"), this), row, 0);
    controlLayout->addWidget(uncheckedColorButton, row, 1);

    // 第四行：颜色控制 - 滑块颜色
    row++;
    controlLayout->addWidget(new QLabel(tr("Thumb color:"), this), row, 0);
    controlLayout->addWidget(thumbColorButton, row, 1);

    // 第五行：颜色控制 - 滑块边框颜色
    row++;
    controlLayout->addWidget(new QLabel(tr("Thumb border color:"), this), row, 0);
    controlLayout->addWidget(thumbBorderColorButton, row, 1);

    // 第六行：动画控制
    row++;
    controlLayout->addWidget(durationLabel, row, 0);
    controlLayout->addWidget(animationDurationSlider, row, 1);

    // 主布局组装
    mainLayout->addWidget(switchButton);
    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(statusLabel);

    setCentralWidget(mainWidget);
    resize(300, 500);
    setWindowTitle(tr("Switch Button Example"));

    // ========== 颜色设置部分 ==========

    // 统一的颜色按钮更新函数
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

        // 根据WCAG标准选择对比度足够的文字颜色
        QString textColor = luminance > 0.179 ? "black" : "white";

        button->setStyleSheet(
            QString("background-color: %1; color: %2; border: 1px solid gray; padding: 5px;")
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
        // 阻塞信号避免循环
        stateCheckbox->blockSignals(true);
        switchButton->setChecked(checked);
        stateCheckbox->blockSignals(false);
    });

    // 颜色设置
    connect(checkedColorButton,
            &QPushButton::clicked,
            this,
            [this, switchButton, checkedColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(switchButton->checkedColor(),
                                                      this,
                                                      tr("Select Checked Color"));
                if (color.isValid()) {
                    switchButton->setCheckedColor(color);
                    updateColorButton(checkedColorButton, color);
                }
            });

    connect(uncheckedColorButton,
            &QPushButton::clicked,
            this,
            [this, switchButton, uncheckedColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(switchButton->uncheckedColor(),
                                                      this,
                                                      tr("Select Unchecked Color"));
                if (color.isValid()) {
                    switchButton->setUncheckedColor(color);
                    updateColorButton(uncheckedColorButton, color);
                }
            });

    connect(thumbColorButton,
            &QPushButton::clicked,
            this,
            [this, switchButton, thumbColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(switchButton->thumbColor(),
                                                      this,
                                                      tr("Select Thumb Color"));
                if (color.isValid()) {
                    switchButton->setThumbColor(color);
                    updateColorButton(thumbColorButton, color);
                }
            });

    connect(thumbBorderColorButton,
            &QPushButton::clicked,
            this,
            [this, switchButton, thumbBorderColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(switchButton->thumbBorderColor(),
                                                      this,
                                                      tr("Select Thumb Border Color"));
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

    // 开关按钮信号连接 - 更新UI状态
    connect(switchButton, &SwitchButton::toggled, this, [stateCheckbox, statusLabel](bool checked) {
        // 更新状态复选框
        stateCheckbox->blockSignals(true);
        stateCheckbox->setChecked(checked);
        stateCheckbox->blockSignals(false);

        // 更新状态标签
        if (checked) {
            statusLabel->setText(tr("Status: Checked"));
            statusLabel->setStyleSheet("color: green;");
        } else {
            statusLabel->setText(tr("Status: Unchecked"));
            statusLabel->setStyleSheet("");
        }
    });

    connect(switchButton, &SwitchButton::animationStarted, this, [statusLabel](bool checked) {
        QString state = checked ? "Checked" : "Unchecked";
        statusLabel->setText(tr("Animating to: %1").arg(state));
        statusLabel->setStyleSheet("color: orange;");
    });

    connect(switchButton, &SwitchButton::animationFinished, this, [statusLabel](bool checked) {
        QString state = checked ? "Checked" : "Unchecked";
        statusLabel->setText(tr("Animation finished: %1").arg(state));
        statusLabel->setStyleSheet(checked ? "color: green;" : "");
    });

    // 初始化状态
    switchButton->setChecked(false);
}

MainWindow::~MainWindow() {}
