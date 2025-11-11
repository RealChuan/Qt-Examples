#include "mainwindow.h"
#include "batterywidget.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建电池控件
    auto *battery = new BatteryWidget(this);

    // 创建数值控制
    auto *valueSlider = new QSlider(Qt::Horizontal, this);
    valueSlider->setRange(0, 100);
    valueSlider->setValue(75);

    auto *valueSpinBox = new QSpinBox(this);
    valueSpinBox->setRange(0, 100);
    valueSpinBox->setValue(75);
    valueSpinBox->setSuffix("%");

    // 创建报警阈值控制
    auto *alarmLabel = new QLabel(tr("Alarm threshold:"), this);
    auto *alarmSlider = new QSlider(Qt::Horizontal, this);
    alarmSlider->setRange(0, 50);
    alarmSlider->setValue(20);
    auto *alarmSpinBox = new QSpinBox(this);
    alarmSpinBox->setRange(0, 50);
    alarmSpinBox->setValue(20);
    alarmSpinBox->setSuffix("%");

    // 创建充电状态控制
    auto *chargingCheckbox = new QCheckBox(tr("Charging state"), this);
    chargingCheckbox->setChecked(false);

    // 创建颜色选择控件
    auto *powerColorButton = new QPushButton(this);
    auto *alarmColorButton = new QPushButton(this);
    auto *borderColorButton = new QPushButton(this);

    // 创建动画控制
    auto *animationDurationSlider = new QSlider(Qt::Horizontal, this);
    animationDurationSlider->setRange(100, 2000);
    animationDurationSlider->setValue(500);
    auto *durationLabel = new QLabel(tr("Animation duration: 500ms"), this);

    // 创建操作按钮
    auto *increaseButton = new QPushButton(tr("+10"), this);
    auto *decreaseButton = new QPushButton(tr("-10"), this);
    auto *resetButton = new QPushButton(tr("Reset"), this);
    auto *animateToButton = new QPushButton(tr("Animate to 50"), this);

    // 创建状态显示标签
    auto *statusLabel = new QLabel(tr("Status: Normal"), this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::Box);
    statusLabel->setMinimumHeight(30);

    // 布局设置 - 针对横向电池优化
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    // 顶部：电池显示区域
    auto *batteryContainer = new QWidget(this);
    auto *batteryLayout = new QHBoxLayout(batteryContainer);
    batteryLayout->addStretch();
    batteryLayout->addWidget(battery);
    batteryLayout->addStretch();

    // 控制面板区域
    auto *controlPanel = new QWidget(this);
    auto *controlLayout = new QGridLayout(controlPanel);

    // 第一行：数值控制
    int row = 0;
    controlLayout->addWidget(new QLabel(tr("Current value:"), this), row, 0);
    controlLayout->addWidget(valueSlider, row, 1);
    controlLayout->addWidget(valueSpinBox, row, 2);

    // 第二行：报警阈值
    row++;
    controlLayout->addWidget(alarmLabel, row, 0);
    controlLayout->addWidget(alarmSlider, row, 1);
    controlLayout->addWidget(alarmSpinBox, row, 2);

    // 第三行：颜色控制
    row++;
    controlLayout->addWidget(new QLabel(tr("Power color:"), this), row, 0);
    controlLayout->addWidget(powerColorButton, row, 1);

    row++;
    controlLayout->addWidget(new QLabel(tr("Alarm color:"), this), row, 0);
    controlLayout->addWidget(alarmColorButton, row, 1);

    row++;
    controlLayout->addWidget(new QLabel(tr("Border color:"), this), row, 0);
    controlLayout->addWidget(borderColorButton, row, 1);

    // 第四行：动画控制
    row++;
    controlLayout->addWidget(durationLabel, row, 0);
    controlLayout->addWidget(animationDurationSlider, row, 1);

    // 第五行：操作按钮
    row++;
    controlLayout->addWidget(new QLabel(tr("Actions:"), this), row, 0);
    controlLayout->addWidget(increaseButton, row, 1);
    controlLayout->addWidget(animateToButton, row, 2);

    row++;
    controlLayout->addWidget(decreaseButton, row, 1);
    controlLayout->addWidget(resetButton, row, 2);

    // 第六行：充电控制
    row++;
    controlLayout->addWidget(new QLabel(tr("Charging:"), this), row, 0);
    controlLayout->addWidget(chargingCheckbox, row, 1);

    // 主布局组装
    mainLayout->addWidget(batteryContainer);
    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(statusLabel);

    setCentralWidget(mainWidget);
    resize(600, 450); // 更适合横向电池的窗口大小
    setWindowTitle(tr("Battery Widget Example"));

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
    updateColorButton(powerColorButton, battery->powerColor());
    updateColorButton(alarmColorButton, battery->alarmColor());
    updateColorButton(borderColorButton, battery->borderColor());

    // ========== 信号连接 ==========

    // 数值控制 - 避免循环信号
    connect(valueSlider, &QSlider::valueChanged, this, [battery, valueSpinBox](int value) {
        // 阻塞spinbox信号，避免循环
        valueSpinBox->blockSignals(true);
        battery->setValue(value);
        valueSpinBox->setValue(value);
        valueSpinBox->blockSignals(false);
    });

    connect(valueSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            [battery, valueSlider](int value) {
                // 阻塞slider信号，避免循环
                valueSlider->blockSignals(true);
                battery->setValue(value);
                valueSlider->setValue(value);
                valueSlider->blockSignals(false);
            });

    // 报警阈值控制 - 避免循环信号
    connect(alarmSlider, &QSlider::valueChanged, this, [battery, alarmSpinBox](int value) {
        alarmSpinBox->blockSignals(true);
        battery->setAlarmValue(value);
        alarmSpinBox->setValue(value);
        alarmSpinBox->blockSignals(false);
    });

    connect(alarmSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            [battery, alarmSlider](int value) {
                alarmSlider->blockSignals(true);
                battery->setAlarmValue(value);
                alarmSlider->setValue(value);
                alarmSlider->blockSignals(false);
            });

    // 充电状态
    connect(chargingCheckbox, &QCheckBox::toggled, battery, &BatteryWidget::setCharging);

    // 颜色设置
    connect(powerColorButton,
            &QPushButton::clicked,
            this,
            [this, battery, powerColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(battery->powerColor(),
                                                      this,
                                                      tr("Select Power Color"));
                if (color.isValid()) {
                    battery->setPowerColor(color);
                    updateColorButton(powerColorButton, color);
                }
            });

    connect(alarmColorButton,
            &QPushButton::clicked,
            this,
            [this, battery, alarmColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(battery->alarmColor(),
                                                      this,
                                                      tr("Select Alarm Color"));
                if (color.isValid()) {
                    battery->setAlarmColor(color);
                    updateColorButton(alarmColorButton, color);
                }
            });

    connect(borderColorButton,
            &QPushButton::clicked,
            this,
            [this, battery, borderColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(battery->borderColor(),
                                                      this,
                                                      tr("Select Border Color"));
                if (color.isValid()) {
                    battery->setBorderColor(color);
                    updateColorButton(borderColorButton, color);
                }
            });

    // 动画设置
    connect(animationDurationSlider,
            &QSlider::valueChanged,
            this,
            [battery, durationLabel](int value) {
                battery->setAnimationDuration(value);
                durationLabel->setText(tr("Animation duration: %1ms").arg(value));
            });

    // 操作按钮
    connect(increaseButton, &QPushButton::clicked, this, [battery]() {
        battery->increaseValue(10);
    });

    connect(decreaseButton, &QPushButton::clicked, this, [battery]() {
        battery->decreaseValue(10);
    });

    connect(resetButton, &QPushButton::clicked, battery, &BatteryWidget::reset);

    connect(animateToButton, &QPushButton::clicked, this, [battery]() {
        battery->setValueAnimated(50);
    });

    // 电池信号连接 - 更新UI状态
    connect(battery, &BatteryWidget::valueChanged, this, [valueSlider, valueSpinBox](int value) {
        valueSlider->blockSignals(true);
        valueSlider->setValue(value);
        valueSlider->blockSignals(false);

        valueSpinBox->blockSignals(true);
        valueSpinBox->setValue(value);
        valueSpinBox->blockSignals(false);
    });

    connect(battery, &BatteryWidget::valueIncreased, this, [](int newValue) {
        qDebug() << "Value increased to:" << newValue;
    });

    connect(battery, &BatteryWidget::valueDecreased, this, [](int newValue) {
        qDebug() << "Value decreased to:" << newValue;
    });

    connect(battery, &BatteryWidget::valueReset, this, []() { qDebug() << "Value reset"; });

    connect(battery,
            &BatteryWidget::animationStarted,
            this,
            [statusLabel](int oldValue, int newValue) {
                statusLabel->setText(tr("Animating: %1% → %2%").arg(oldValue).arg(newValue));
                statusLabel->setStyleSheet("color: orange;");
            });

    connect(battery, &BatteryWidget::animationFinished, this, [statusLabel](int value) {
        statusLabel->setText(tr("Animation finished: %1%").arg(value));
        statusLabel->setStyleSheet("color: green;");
    });

    connect(battery, &BatteryWidget::alarmStateChanged, this, [statusLabel](bool isAlarm) {
        if (isAlarm) {
            statusLabel->setText(tr("Status: Low battery alarm!"));
            statusLabel->setStyleSheet("color: red; font-weight: bold;");
        } else {
            statusLabel->setText(tr("Status: Normal"));
            statusLabel->setStyleSheet("");
        }
    });

    connect(battery,
            &BatteryWidget::chargingChanged,
            this,
            [statusLabel, chargingCheckbox](bool charging) {
                chargingCheckbox->blockSignals(true);
                chargingCheckbox->setChecked(charging);
                chargingCheckbox->blockSignals(false);

                if (charging) {
                    statusLabel->setText(tr("Status: Charging..."));
                    statusLabel->setStyleSheet("color: blue;");
                }
            });

    // 初始化状态
    battery->setValue(75);
}

MainWindow::~MainWindow() {}
