#include "mainwindow.h"
#include "clockwidget.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建时钟控件
    auto *clock = new ClockWidget(this);

    // 创建颜色选择控件 - 使用颜色预览按钮
    auto *borderColorLabel = new QLabel(tr("Border color:"), this);
    auto *borderColorButton = new QPushButton(this);

    auto *backgroundColorLabel = new QLabel(tr("Background color:"), this);
    auto *backgroundColorButton = new QPushButton(this);

    auto *hourColorLabel = new QLabel(tr("Hour hand color:"), this);
    auto *hourColorButton = new QPushButton(this);

    auto *minuteColorLabel = new QLabel(tr("Minute hand color:"), this);
    auto *minuteColorButton = new QPushButton(this);

    auto *secondColorLabel = new QLabel(tr("Second hand color:"), this);
    auto *secondColorButton = new QPushButton(this);

    auto *textColorLabel = new QLabel(tr("Text color:"), this);
    auto *textColorButton = new QPushButton(this);

    // 创建动画控制
    auto *smoothAnimationCheckbox = new QCheckBox(tr("Smooth animation"), this);
    smoothAnimationCheckbox->setChecked(true);
    auto *showSecondsCheckbox = new QCheckBox(tr("Show seconds"), this);
    showSecondsCheckbox->setChecked(true);

    // 创建动画时长控制
    auto *animationDurationSlider = new QSlider(Qt::Horizontal, this);
    animationDurationSlider->setRange(50, 1000);
    animationDurationSlider->setValue(200);
    auto *durationLabel = new QLabel(tr("Animation duration: 200ms"), this);

    // 创建时间控制
    auto *timeLabel = new QLabel(tr("Current time: "), this);
    timeLabel->setAlignment(Qt::AlignCenter);
    auto *updateTimeButton = new QPushButton(tr("Update time"), this);
    auto *toggleAnimationButton = new QPushButton(tr("Toggle animation"), this);

    // 创建快速操作按钮
    auto *resetColorsButton = new QPushButton(tr("Reset colors"), this);
    auto *themeGroup = new QGroupBox(tr("Quick themes"), this);
    auto *classicThemeButton = new QPushButton(tr("Classic"), this);
    auto *darkThemeButton = new QPushButton(tr("Dark"), this);
    auto *modernThemeButton = new QPushButton(tr("Modern"), this);

    // 布局设置
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(mainWidget);

    // 左侧：时钟显示
    auto *displayLayout = new QVBoxLayout();
    displayLayout->addWidget(clock, 1);
    displayLayout->addWidget(timeLabel);
    displayLayout->setAlignment(timeLabel, Qt::AlignCenter);

    // 右侧：控制面板
    auto *controlPanel = new QWidget(this);
    auto *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setAlignment(Qt::AlignTop);

    // 颜色控制布局
    auto *colorGroup = new QGroupBox(tr("Color settings"), this);
    auto *colorLayout = new QGridLayout(colorGroup);

    // 第一列颜色设置
    colorLayout->addWidget(borderColorLabel, 0, 0);
    colorLayout->addWidget(borderColorButton, 0, 1);
    colorLayout->addWidget(backgroundColorLabel, 1, 0);
    colorLayout->addWidget(backgroundColorButton, 1, 1);
    colorLayout->addWidget(hourColorLabel, 2, 0);
    colorLayout->addWidget(hourColorButton, 2, 1);

    // 第二列颜色设置
    colorLayout->addWidget(minuteColorLabel, 0, 2);
    colorLayout->addWidget(minuteColorButton, 0, 3);
    colorLayout->addWidget(secondColorLabel, 1, 2);
    colorLayout->addWidget(secondColorButton, 1, 3);
    colorLayout->addWidget(textColorLabel, 2, 2);
    colorLayout->addWidget(textColorButton, 2, 3);

    // 快速主题布局
    auto *themeLayout = new QHBoxLayout(themeGroup);
    themeLayout->addWidget(classicThemeButton);
    themeLayout->addWidget(darkThemeButton);
    themeLayout->addWidget(modernThemeButton);

    // 动画控制布局
    auto *animationGroup = new QGroupBox(tr("Animation settings"), this);
    auto *animationLayout = new QVBoxLayout(animationGroup);
    animationLayout->addWidget(smoothAnimationCheckbox);
    animationLayout->addWidget(showSecondsCheckbox);
    animationLayout->addWidget(durationLabel);
    animationLayout->addWidget(animationDurationSlider);

    // 时间控制布局
    auto *timeGroup = new QGroupBox(tr("Time control"), this);
    auto *timeLayout = new QHBoxLayout(timeGroup);
    timeLayout->addWidget(updateTimeButton);
    timeLayout->addWidget(toggleAnimationButton);

    // 操作按钮布局
    auto *actionsLayout = new QHBoxLayout();
    actionsLayout->addWidget(resetColorsButton);

    // 组装控制面板
    controlLayout->addWidget(colorGroup);
    controlLayout->addWidget(themeGroup);
    controlLayout->addWidget(animationGroup);
    controlLayout->addWidget(timeGroup);
    controlLayout->addLayout(actionsLayout);

    // 主布局
    mainLayout->addLayout(displayLayout, 2); // 时钟区域占更多空间
    mainLayout->addWidget(controlPanel, 1);

    setCentralWidget(mainWidget);
    resize(800, 500);
    setWindowTitle(tr("Clock Widget Example"));

    // 初始化颜色按钮
    auto updateColorButton = [](QPushButton *button, const QColor &color) {
        button->setStyleSheet(
            QString("background-color: %1; border: 1px solid gray;").arg(color.name()));
    };

    // 设置初始颜色
    updateColorButton(borderColorButton, clock->borderColor());
    updateColorButton(backgroundColorButton, clock->backgroundColor());
    updateColorButton(hourColorButton, clock->hourColor());
    updateColorButton(minuteColorButton, clock->minuteColor());
    updateColorButton(secondColorButton, clock->secondColor());
    updateColorButton(textColorButton, clock->textColor());

    // 连接信号和槽

    // 颜色设置 - 使用统一的颜色选择处理
    auto setupColorConnection = [this, updateColorButton](QPushButton *button,
                                                          std::function<void(const QColor &)> setter,
                                                          std::function<QColor()> getter) {
        connect(button,
                &QPushButton::clicked,
                this,
                [this, button, setter, getter, updateColorButton]() {
                    QColor color = QColorDialog::getColor(getter(), this, tr("Select color"));
                    if (color.isValid()) {
                        setter(color);
                        updateColorButton(button, color);
                    }
                });
    };

    setupColorConnection(
        borderColorButton,
        [clock](const QColor &color) { clock->setBorderColor(color); },
        [clock]() { return clock->borderColor(); });

    setupColorConnection(
        backgroundColorButton,
        [clock](const QColor &color) { clock->setBackgroundColor(color); },
        [clock]() { return clock->backgroundColor(); });

    setupColorConnection(
        hourColorButton,
        [clock](const QColor &color) { clock->setHourColor(color); },
        [clock]() { return clock->hourColor(); });

    setupColorConnection(
        minuteColorButton,
        [clock](const QColor &color) { clock->setMinuteColor(color); },
        [clock]() { return clock->minuteColor(); });

    setupColorConnection(
        secondColorButton,
        [clock](const QColor &color) { clock->setSecondColor(color); },
        [clock]() { return clock->secondColor(); });

    setupColorConnection(
        textColorButton,
        [clock](const QColor &color) { clock->setTextColor(color); },
        [clock]() { return clock->textColor(); });

    // 动画设置
    connect(smoothAnimationCheckbox, &QCheckBox::toggled, clock, &ClockWidget::setSmoothAnimation);
    connect(showSecondsCheckbox, &QCheckBox::toggled, clock, &ClockWidget::setShowSeconds);

    connect(animationDurationSlider, &QSlider::valueChanged, this, [clock, durationLabel](int value) {
        clock->setAnimationDuration(value);
        durationLabel->setText(tr("Animation duration: %1ms").arg(value));
    });

    // 时间控制
    connect(updateTimeButton, &QPushButton::clicked, clock, &ClockWidget::updateTime);
    connect(toggleAnimationButton, &QPushButton::clicked, clock, [smoothAnimationCheckbox, clock]() {
        bool currentState = !smoothAnimationCheckbox->isChecked();
        smoothAnimationCheckbox->setChecked(currentState);
        clock->setSmoothAnimation(currentState);
    });

    // 快速主题
    connect(classicThemeButton,
            &QPushButton::clicked,
            this,
            [clock,
             updateColorButton,
             borderColorButton,
             backgroundColorButton,
             hourColorButton,
             minuteColorButton,
             secondColorButton,
             textColorButton]() {
                // 设置经典主题颜色
                clock->setBorderColor(Qt::black);
                clock->setBackgroundColor(Qt::white);
                clock->setHourColor(Qt::black);
                clock->setMinuteColor(Qt::darkGray);
                clock->setSecondColor(Qt::red);
                clock->setTextColor(Qt::black);

                // 更新按钮颜色
                updateColorButton(borderColorButton, Qt::black);
                updateColorButton(backgroundColorButton, Qt::white);
                updateColorButton(hourColorButton, Qt::black);
                updateColorButton(minuteColorButton, Qt::darkGray);
                updateColorButton(secondColorButton, Qt::red);
                updateColorButton(textColorButton, Qt::black);
            });

    connect(darkThemeButton,
            &QPushButton::clicked,
            this,
            [clock,
             updateColorButton,
             borderColorButton,
             backgroundColorButton,
             hourColorButton,
             minuteColorButton,
             secondColorButton,
             textColorButton]() {
                // 设置暗色主题颜色
                clock->setBorderColor(Qt::white);
                clock->setBackgroundColor(Qt::darkGray);
                clock->setHourColor(Qt::white);
                clock->setMinuteColor(Qt::lightGray);
                clock->setSecondColor(Qt::yellow);
                clock->setTextColor(Qt::white);

                // 更新按钮颜色
                updateColorButton(borderColorButton, Qt::white);
                updateColorButton(backgroundColorButton, Qt::darkGray);
                updateColorButton(hourColorButton, Qt::white);
                updateColorButton(minuteColorButton, Qt::lightGray);
                updateColorButton(secondColorButton, Qt::yellow);
                updateColorButton(textColorButton, Qt::white);
            });

    connect(modernThemeButton,
            &QPushButton::clicked,
            this,
            [clock,
             updateColorButton,
             borderColorButton,
             backgroundColorButton,
             hourColorButton,
             minuteColorButton,
             secondColorButton,
             textColorButton]() {
                // 设置现代主题颜色
                clock->setBorderColor(QColor(70, 130, 180));      // Steel Blue
                clock->setBackgroundColor(QColor(240, 248, 255)); // Alice Blue
                clock->setHourColor(QColor(25, 25, 112));         // Midnight Blue
                clock->setMinuteColor(QColor(65, 105, 225));      // Royal Blue
                clock->setSecondColor(QColor(220, 20, 60));       // Crimson
                clock->setTextColor(QColor(47, 79, 79));          // Dark Slate Gray

                // 更新按钮颜色
                updateColorButton(borderColorButton, QColor(70, 130, 180));
                updateColorButton(backgroundColorButton, QColor(240, 248, 255));
                updateColorButton(hourColorButton, QColor(25, 25, 112));
                updateColorButton(minuteColorButton, QColor(65, 105, 225));
                updateColorButton(secondColorButton, QColor(220, 20, 60));
                updateColorButton(textColorButton, QColor(47, 79, 79));
            });

    // 重置颜色
    connect(resetColorsButton,
            &QPushButton::clicked,
            this,
            [clock,
             updateColorButton,
             borderColorButton,
             backgroundColorButton,
             hourColorButton,
             minuteColorButton,
             secondColorButton,
             textColorButton]() {
                // 获取默认颜色（假设ClockWidget有默认颜色）
                QColor defaultBorder = Qt::black;
                QColor defaultBackground = Qt::white;
                QColor defaultHour = Qt::black;
                QColor defaultMinute = Qt::black;
                QColor defaultSecond = Qt::red;
                QColor defaultText = Qt::black;

                clock->setBorderColor(defaultBorder);
                clock->setBackgroundColor(defaultBackground);
                clock->setHourColor(defaultHour);
                clock->setMinuteColor(defaultMinute);
                clock->setSecondColor(defaultSecond);
                clock->setTextColor(defaultText);

                updateColorButton(borderColorButton, defaultBorder);
                updateColorButton(backgroundColorButton, defaultBackground);
                updateColorButton(hourColorButton, defaultHour);
                updateColorButton(minuteColorButton, defaultMinute);
                updateColorButton(secondColorButton, defaultSecond);
                updateColorButton(textColorButton, defaultText);
            });

    // 时间更新信号
    connect(clock, &ClockWidget::timeUpdated, this, [timeLabel](const QTime &time) {
        timeLabel->setText(tr("Current time: %1").arg(time.toString("hh:mm:ss")));
    });

    // 初始化状态
    clock->updateTime();
    QTimer::singleShot(100, [timeLabel]() {
        timeLabel->setText(tr("Current time: %1").arg(QTime::currentTime().toString("hh:mm:ss")));
    });
}

MainWindow::~MainWindow() {}
