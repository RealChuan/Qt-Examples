#include "mainwindow.h"
#include "clockwidget.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建时钟控件
    auto *clock = new ClockWidget(this);

    // 创建颜色选择控件
    auto *borderColorButton = new QPushButton(this);
    auto *backgroundColorButton = new QPushButton(this);
    auto *hourColorButton = new QPushButton(this);
    auto *minuteColorButton = new QPushButton(this);
    auto *secondColorButton = new QPushButton(this);
    auto *textColorButton = new QPushButton(this);

    // 创建动画控制
    auto *smoothSecondsCheckbox = new QCheckBox(tr("Smooth seconds"), this);
    smoothSecondsCheckbox->setChecked(true);
    auto *showSecondsCheckbox = new QCheckBox(tr("Show seconds"), this);
    showSecondsCheckbox->setChecked(true);

    // 创建时间控制
    auto *timeLabel = new QLabel(tr("Current time: "), this);
    timeLabel->setAlignment(Qt::AlignCenter);
    auto *updateTimeButton = new QPushButton(tr("Update time"), this);

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
    animationLayout->addWidget(smoothSecondsCheckbox);
    animationLayout->addWidget(showSecondsCheckbox);

    // 操作按钮布局
    auto *actionsLayout = new QHBoxLayout();
    actionsLayout->addWidget(updateTimeButton);
    actionsLayout->addWidget(resetColorsButton);

    // 组装控制面板
    controlLayout->addWidget(colorGroup);
    controlLayout->addWidget(themeGroup);
    controlLayout->addWidget(animationGroup);
    controlLayout->addLayout(actionsLayout);

    // 主布局
    mainLayout->addLayout(displayLayout, 2);
    mainLayout->addWidget(controlPanel, 1);

    setCentralWidget(mainWidget);
    resize(800, 350);
    setWindowTitle(tr("Clock Widget Example"));

    // 初始化颜色按钮
    auto updateColorButton = [](QPushButton *button, const QColor &color) {
        QString colorName = color.name(QColor::HexArgb).toUpper();

        // 计算相对亮度
        auto getRelativeLuminance = [](int r, int g, int b) {
            auto normalize = [](double x) {
                return x <= 0.03928 ? x / 12.92 : std::pow((x + 0.055) / 1.055, 2.4);
            };
            return 0.2126 * normalize(r / 255.0) + 0.7152 * normalize(g / 255.0)
                   + 0.0722 * normalize(b / 255.0);
        };

        double luminance = getRelativeLuminance(color.red(), color.green(), color.blue());
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

    // 颜色设置
    auto createColorDialogConnector =
        [this, updateColorButton](QPushButton *button,
                                  std::function<QColor()> getColorFunc,
                                  std::function<void(const QColor &)> setColorFunc,
                                  const QString &title) {
            connect(button,
                    &QPushButton::clicked,
                    this,
                    [this, button, getColorFunc, setColorFunc, title, updateColorButton]() {
                        QColor color = QColorDialog::getColor(getColorFunc(), this, title);
                        if (!color.isValid())
                            return;

                        setColorFunc(color);
                        updateColorButton(button, color);
                    });
        };

    createColorDialogConnector(
        borderColorButton,
        [clock]() { return clock->borderColor(); },
        [clock](const QColor &color) { clock->setBorderColor(color); },
        tr("Select border color"));

    createColorDialogConnector(
        backgroundColorButton,
        [clock]() { return clock->backgroundColor(); },
        [clock](const QColor &color) { clock->setBackgroundColor(color); },
        tr("Select background color"));

    createColorDialogConnector(
        hourColorButton,
        [clock]() { return clock->hourColor(); },
        [clock](const QColor &color) { clock->setHourColor(color); },
        tr("Select hour hand color"));

    createColorDialogConnector(
        minuteColorButton,
        [clock]() { return clock->minuteColor(); },
        [clock](const QColor &color) { clock->setMinuteColor(color); },
        tr("Select minute hand color"));

    createColorDialogConnector(
        secondColorButton,
        [clock]() { return clock->secondColor(); },
        [clock](const QColor &color) { clock->setSecondColor(color); },
        tr("Select second hand color"));

    createColorDialogConnector(
        textColorButton,
        [clock]() { return clock->textColor(); },
        [clock](const QColor &color) { clock->setTextColor(color); },
        tr("Select text color"));

    // 动画设置
    connect(smoothSecondsCheckbox, &QCheckBox::toggled, clock, &ClockWidget::setSmoothSeconds);
    connect(showSecondsCheckbox, &QCheckBox::toggled, clock, &ClockWidget::setShowSeconds);

    // 时间控制
    connect(updateTimeButton, &QPushButton::clicked, clock, &ClockWidget::updateClock);

    // 保存默认颜色用于重置
    QColor defaultBorder = clock->borderColor();
    QColor defaultBackground = clock->backgroundColor();
    QColor defaultHour = clock->hourColor();
    QColor defaultMinute = clock->minuteColor();
    QColor defaultSecond = clock->secondColor();
    QColor defaultText = clock->textColor();

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
                clock->setBorderColor(QColor(80, 80, 80));
                clock->setBackgroundColor(QColor(50, 50, 50));
                clock->setHourColor(QColor(240, 240, 240));
                clock->setMinuteColor(QColor(220, 220, 220));
                clock->setSecondColor(QColor(255, 80, 80));
                clock->setTextColor(QColor(240, 240, 240));

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
                clock->setBorderColor(Qt::white);
                clock->setBackgroundColor(Qt::darkGray);
                clock->setHourColor(Qt::white);
                clock->setMinuteColor(Qt::lightGray);
                clock->setSecondColor(Qt::yellow);
                clock->setTextColor(Qt::white);

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
                clock->setBorderColor(QColor(70, 130, 180));
                clock->setBackgroundColor(QColor(240, 248, 255));
                clock->setHourColor(QColor(25, 25, 112));
                clock->setMinuteColor(QColor(65, 105, 225));
                clock->setSecondColor(QColor(220, 20, 60));
                clock->setTextColor(QColor(47, 79, 79));

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
             textColorButton,
             defaultBorder,
             defaultBackground,
             defaultHour,
             defaultMinute,
             defaultSecond,
             defaultText]() {
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

    // 初始化时间显示
    clock->updateClock();
}

MainWindow::~MainWindow() {}
