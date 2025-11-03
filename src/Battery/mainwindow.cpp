#include "mainwindow.h"
#include "batterywidget.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建电池控件
    auto *battery = new BatteryWidget(this);
    battery->setValue(75); // 设置初始电量

    // 创建滑动条控制电量
    auto *slider = new QSlider(Qt::Vertical, this);
    slider->setRange(0, 100);
    slider->setValue(75);

    // 创建充电控制复选框
    auto *chargingCheckbox = new QCheckBox(tr("Charging state"), this);
    chargingCheckbox->setChecked(false);

    // 创建报警阈值设置
    auto *alarmSlider = new QSlider(Qt::Horizontal, this);
    alarmSlider->setRange(0, 50);
    alarmSlider->setValue(20);
    auto *alarmLabel = new QLabel(tr("Alarm threshold: 20%"), this);

    // 创建颜色选择控件 - 使用颜色预览按钮
    auto *powerColorLabel = new QLabel(tr("Battery color:"), this);
    auto *powerColorButton = new QPushButton(this);

    auto *alarmColorLabel = new QLabel(tr("Alarm color:"), this);
    auto *alarmColorButton = new QPushButton(this);

    auto *borderColorLabel = new QLabel(tr("Border color:"), this);
    auto *borderColorButton = new QPushButton(this);

    // 创建动画控制
    auto *animationCheckbox = new QCheckBox(tr("Enable animation"), this);
    animationCheckbox->setChecked(true);
    auto *animationDurationSlider = new QSlider(Qt::Horizontal, this);
    animationDurationSlider->setRange(100, 2000);
    animationDurationSlider->setValue(500);
    auto *durationLabel = new QLabel(tr("Animation duration: 500ms"), this);

    // 创建快速操作按钮
    auto *increaseButton = new QPushButton("+10%", this);
    auto *decreaseButton = new QPushButton("-10%", this);
    auto *resetButton = new QPushButton(tr("Reset"), this);

    // 创建状态显示标签
    auto *statusLabel = new QLabel(tr("Status: Normal"), this);
    statusLabel->setAlignment(Qt::AlignCenter);

    // 布局设置
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    // 电池和滑动条布局
    auto *batteryLayout = new QHBoxLayout();
    batteryLayout->addWidget(battery);
    batteryLayout->addWidget(slider);

    // 充电和动画控制布局
    auto *gridLayout = new QGridLayout;
    gridLayout->addWidget(chargingCheckbox, 0, 0);
    gridLayout->addWidget(animationCheckbox, 0, 1);
    gridLayout->addWidget(powerColorLabel, 1, 0);
    gridLayout->addWidget(powerColorButton, 1, 1);
    gridLayout->addWidget(alarmColorLabel, 2, 0);
    gridLayout->addWidget(alarmColorButton, 2, 1);
    gridLayout->addWidget(borderColorLabel, 3, 0);
    gridLayout->addWidget(borderColorButton, 3, 1);
    gridLayout->addWidget(alarmLabel, 4, 0);
    gridLayout->addWidget(alarmSlider, 4, 1);
    gridLayout->addWidget(durationLabel, 5, 0);
    gridLayout->addWidget(animationDurationSlider, 5, 1);

    // 快速操作布局
    auto *quickActionsLayout = new QHBoxLayout();
    quickActionsLayout->addWidget(increaseButton);
    quickActionsLayout->addWidget(decreaseButton);
    quickActionsLayout->addWidget(resetButton);

    // 主布局组装
    mainLayout->addLayout(batteryLayout);
    mainLayout->addLayout(gridLayout);
    mainLayout->addLayout(quickActionsLayout);
    mainLayout->addWidget(statusLabel);

    setCentralWidget(mainWidget);
    resize(300, 350);
    setWindowTitle(tr("Battery Widget Example"));

    // 连接信号和槽

    // 电量控制
    connect(slider, &QSlider::valueChanged, battery, [battery, animationCheckbox](int value) {
        if (animationCheckbox->isChecked()) {
            battery->setValueAnimated(value);
        } else {
            battery->setValue(value);
        }
    });

    // 充电状态
    connect(chargingCheckbox, &QCheckBox::toggled, battery, &BatteryWidget::setCharging);

    // 报警阈值
    connect(alarmSlider, &QSlider::valueChanged, battery, [battery, alarmLabel](int value) {
        battery->setAlarmValue(value);
        alarmLabel->setText(tr("Alarm threshold: %1%").arg(value));
    });

    // 动画控制
    connect(animationDurationSlider,
            &QSlider::valueChanged,
            battery,
            [battery, durationLabel](int value) {
                battery->setAnimationDuration(value);
                durationLabel->setText(tr("Animation duration: %1ms").arg(value));
            });

    // 颜色选择 - 更新按钮颜色预览
    auto updateColorButton = [](QPushButton *button, const QColor &color) {
        button->setStyleSheet(
            QString("background-color: %1; border: 1px solid gray;").arg(color.name()));
    };

    updateColorButton(powerColorButton, battery->powerColor());
    updateColorButton(alarmColorButton, battery->alarmColor());
    updateColorButton(borderColorButton, battery->borderColor());

    connect(powerColorButton,
            &QPushButton::clicked,
            this,
            [this, battery, powerColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(battery->powerColor(),
                                                      this,
                                                      tr("Select the battery color"));
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
                                                      tr("Select the alarm color"));
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
                                                      tr("Select the border color"));
                if (color.isValid()) {
                    battery->setBorderColor(color);
                    updateColorButton(borderColorButton, color);
                }
            });

    // 快速操作
    connect(increaseButton, &QPushButton::clicked, battery, [battery]() {
        battery->increaseValue(10);
    });

    connect(decreaseButton, &QPushButton::clicked, battery, [battery]() {
        battery->decreaseValue(10);
    });

    connect(resetButton, &QPushButton::clicked, battery, &BatteryWidget::reset);

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
                chargingCheckbox->setChecked(charging);
                if (charging) {
                    statusLabel->setText(tr("Status: Charging..."));
                    statusLabel->setStyleSheet("color: blue;");
                }
            });

    connect(battery,
            &BatteryWidget::animationStarted,
            this,
            [statusLabel](int oldValue, int newValue) {
                statusLabel->setText(tr("Animation: %1% → %2%").arg(oldValue).arg(newValue));
            });

    connect(battery, &BatteryWidget::animationFinished, this, [statusLabel](int value) {
        statusLabel->setText(tr("The animation is completed: %1%").arg(value));
    });

    // 初始化状态
    battery->setValue(75);
}

MainWindow::~MainWindow() {}
