#include "mainwindow.h"
#include "clockwidget.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建时钟控件
    auto *clock = new ClockWidget(this);

    // 创建颜色选择控件 - 使用颜色预览按钮（类似第一个代码的风格）
    auto *borderColorButton = new QPushButton(this);
    auto *backgroundColorButton = new QPushButton(this);
    auto *hourColorButton = new QPushButton(this);
    auto *minuteColorButton = new QPushButton(this);
    auto *secondColorButton = new QPushButton(this);
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

    // 颜色设置网格布局
    colorLayout->addWidget(new QLabel(tr("Border color:"), this), 0, 0);
    colorLayout->addWidget(borderColorButton, 0, 1);
    colorLayout->addWidget(new QLabel(tr("Background color:"), this), 1, 0);
    colorLayout->addWidget(backgroundColorButton, 1, 1);
    colorLayout->addWidget(new QLabel(tr("Hour hand color:"), this), 2, 0);
    colorLayout->addWidget(hourColorButton, 2, 1);

    colorLayout->addWidget(new QLabel(tr("Minute hand color:"), this), 0, 2);
    colorLayout->addWidget(minuteColorButton, 0, 3);
    colorLayout->addWidget(new QLabel(tr("Second hand color:"), this), 1, 2);
    colorLayout->addWidget(secondColorButton, 1, 3);
    colorLayout->addWidget(new QLabel(tr("Text color:"), this), 2, 2);
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

    // 初始化颜色按钮 - 类似第一个代码的风格
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

    // 设置初始颜色
    updateColorButton(borderColorButton, clock->borderColor());
    updateColorButton(backgroundColorButton, clock->backgroundColor());
    updateColorButton(hourColorButton, clock->hourColor());
    updateColorButton(minuteColorButton, clock->minuteColor());
    updateColorButton(secondColorButton, clock->secondColor());
    updateColorButton(textColorButton, clock->textColor());

    // 连接信号和槽

    // 颜色设置 - 改造为类似第一个代码的简洁风格
    connect(borderColorButton,
            &QPushButton::clicked,
            this,
            [this, clock, borderColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(clock->borderColor(),
                                                      this,
                                                      tr("Select border color"));
                if (color.isValid()) {
                    clock->setBorderColor(color);
                    updateColorButton(borderColorButton, color);
                }
            });

    connect(backgroundColorButton,
            &QPushButton::clicked,
            this,
            [this, clock, backgroundColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(clock->backgroundColor(),
                                                      this,
                                                      tr("Select background color"));
                if (color.isValid()) {
                    clock->setBackgroundColor(color);
                    updateColorButton(backgroundColorButton, color);
                }
            });

    connect(hourColorButton,
            &QPushButton::clicked,
            this,
            [this, clock, hourColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(clock->hourColor(),
                                                      this,
                                                      tr("Select hour hand color"));
                if (color.isValid()) {
                    clock->setHourColor(color);
                    updateColorButton(hourColorButton, color);
                }
            });

    connect(minuteColorButton,
            &QPushButton::clicked,
            this,
            [this, clock, minuteColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(clock->minuteColor(),
                                                      this,
                                                      tr("Select minute hand color"));
                if (color.isValid()) {
                    clock->setMinuteColor(color);
                    updateColorButton(minuteColorButton, color);
                }
            });

    connect(secondColorButton,
            &QPushButton::clicked,
            this,
            [this, clock, secondColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(clock->secondColor(),
                                                      this,
                                                      tr("Select second hand color"));
                if (color.isValid()) {
                    clock->setSecondColor(color);
                    updateColorButton(secondColorButton, color);
                }
            });

    connect(textColorButton,
            &QPushButton::clicked,
            this,
            [this, clock, textColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(clock->textColor(),
                                                      this,
                                                      tr("Select text color"));
                if (color.isValid()) {
                    clock->setTextColor(color);
                    updateColorButton(textColorButton, color);
                }
            });

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
                clock->setBorderColor(QColor(80, 80, 80));
                clock->setBackgroundColor(QColor(50, 50, 50));
                clock->setHourColor(QColor(240, 240, 240));
                clock->setMinuteColor(QColor(220, 220, 220));
                clock->setSecondColor(QColor(255, 80, 80));
                clock->setTextColor(QColor(240, 240, 240));

                // 更新按钮颜色
                updateColorButton(borderColorButton, clock->borderColor());
                updateColorButton(backgroundColorButton, clock->backgroundColor());
                updateColorButton(hourColorButton, clock->hourColor());
                updateColorButton(minuteColorButton, clock->minuteColor());
                updateColorButton(secondColorButton, clock->secondColor());
                updateColorButton(textColorButton, clock->textColor());
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
                updateColorButton(borderColorButton, clock->borderColor());
                updateColorButton(backgroundColorButton, clock->backgroundColor());
                updateColorButton(hourColorButton, clock->hourColor());
                updateColorButton(minuteColorButton, clock->minuteColor());
                updateColorButton(secondColorButton, clock->secondColor());
                updateColorButton(textColorButton, clock->textColor());
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
                updateColorButton(borderColorButton, clock->borderColor());
                updateColorButton(backgroundColorButton, clock->backgroundColor());
                updateColorButton(hourColorButton, clock->hourColor());
                updateColorButton(minuteColorButton, clock->minuteColor());
                updateColorButton(secondColorButton, clock->secondColor());
                updateColorButton(textColorButton, clock->textColor());
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
    QMetaObject::invokeMethod(
        this,
        [timeLabel]() {
            timeLabel->setText(
                tr("Current time: %1").arg(QTime::currentTime().toString("hh:mm:ss")));
        },
        Qt::QueuedConnection);
}

MainWindow::~MainWindow() {}
