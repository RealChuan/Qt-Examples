#include "mainwindow.hpp"
#include "progressbar.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建进度条控件
    auto *progressBar = new ProgressBar(this);

    // 创建数值控制
    auto *valueSlider = new QSlider(Qt::Horizontal, this);
    valueSlider->setRange(0, 100);
    valueSlider->setValue(0);

    auto *valueSpinBox = new QDoubleSpinBox(this);
    valueSpinBox->setRange(0, 100);
    valueSpinBox->setValue(0);
    valueSpinBox->setDecimals(2);

    // 创建范围控制
    auto *minValueLabel = new QLabel(tr("Min value:"), this);
    auto *minValueSpinBox = new QDoubleSpinBox(this);
    minValueSpinBox->setRange(-1000, 1000);
    minValueSpinBox->setValue(0);
    minValueSpinBox->setDecimals(2);

    auto *maxValueLabel = new QLabel(tr("Max value:"), this);
    auto *maxValueSpinBox = new QDoubleSpinBox(this);
    maxValueSpinBox->setRange(-1000, 1000);
    maxValueSpinBox->setValue(100);
    maxValueSpinBox->setDecimals(2);

    // 创建圆角控制
    auto *radiusLabel = new QLabel(tr("Radius:"), this);
    auto *radiusSpinBox = new QDoubleSpinBox(this);
    radiusSpinBox->setRange(0, 100);
    radiusSpinBox->setValue(progressBar->radius());
    radiusSpinBox->setDecimals(1);

    auto *autoRadiusCheck = new QCheckBox(tr("Auto radius"), this);
    autoRadiusCheck->setChecked(progressBar->autoRadius());

    // 创建显示设置
    auto *showPercentCheck = new QCheckBox(tr("Show percentage"), this);
    showPercentCheck->setChecked(true);

    // 创建颜色选择控件
    auto *chunkColorButton = new QPushButton(this);
    auto *textColorButton = new QPushButton(this);
    auto *baseColorButton = new QPushButton(this);
    auto *backgroundColorButton = new QPushButton(this);

    // 创建动画控制
    auto *animationDurationSlider = new QSlider(Qt::Horizontal, this);
    animationDurationSlider->setRange(0, 2000);
    animationDurationSlider->setValue(500);
    auto *durationLabel = new QLabel(tr("Animation duration: 500ms"), this);

    // 创建操作按钮
    auto *increaseButton = new QPushButton(tr("Increase +10"), this);
    auto *decreaseButton = new QPushButton(tr("Decrease -10"), this);
    auto *resetButton = new QPushButton(tr("Reset"), this);
    auto *animateToButton = new QPushButton(tr("Animate to 75"), this);

    // 创建快速主题按钮
    auto *themeGroup = new QGroupBox(tr("Quick themes"), this);
    auto *classicThemeButton = new QPushButton(tr("Classic"), this);
    auto *darkThemeButton = new QPushButton(tr("Dark"), this);
    auto *modernThemeButton = new QPushButton(tr("Modern"), this);
    auto *flatThemeButton = new QPushButton(tr("Flat"), this);

    // 布局设置
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(mainWidget);

    // 右侧：控制面板
    auto *controlPanel = new QWidget(this);
    auto *controlLayout = new QVBoxLayout(controlPanel);

    // 数值控制布局
    auto *valueGroup = new QGroupBox(tr("Value settings"), this);
    auto *valueControlLayout = new QGridLayout(valueGroup);

    valueControlLayout->addWidget(new QLabel(tr("Current value:"), this), 0, 0);
    valueControlLayout->addWidget(valueSlider, 0, 1);
    valueControlLayout->addWidget(valueSpinBox, 0, 2);

    valueControlLayout->addWidget(minValueLabel, 1, 0);
    valueControlLayout->addWidget(minValueSpinBox, 1, 1, 1, 2);

    valueControlLayout->addWidget(maxValueLabel, 2, 0);
    valueControlLayout->addWidget(maxValueSpinBox, 2, 1, 1, 2);

    valueControlLayout->addWidget(showPercentCheck, 3, 0, 1, 3);

    // 圆角控制布局
    auto *radiusGroup = new QGroupBox(tr("Radius settings"), this);
    auto *radiusLayout = new QGridLayout(radiusGroup);

    radiusLayout->addWidget(radiusLabel, 0, 0);
    radiusLayout->addWidget(radiusSpinBox, 0, 1);
    radiusLayout->addWidget(autoRadiusCheck, 1, 0, 1, 2);

    // 颜色控制布局
    auto *colorGroup = new QGroupBox(tr("Color settings"), this);
    auto *colorLayout = new QGridLayout(colorGroup);

    colorLayout->addWidget(new QLabel(tr("Chunk color:"), this), 0, 0);
    colorLayout->addWidget(chunkColorButton, 0, 1);
    colorLayout->addWidget(new QLabel(tr("Text color:"), this), 1, 0);
    colorLayout->addWidget(textColorButton, 1, 1);
    colorLayout->addWidget(new QLabel(tr("Base color:"), this), 0, 2);
    colorLayout->addWidget(baseColorButton, 0, 3);
    colorLayout->addWidget(new QLabel(tr("Background color:"), this), 1, 2);
    colorLayout->addWidget(backgroundColorButton, 1, 3);

    // 动画控制布局
    auto *animationGroup = new QGroupBox(tr("Animation settings"), this);
    auto *animationLayout = new QVBoxLayout(animationGroup);
    animationLayout->addWidget(durationLabel);
    animationLayout->addWidget(animationDurationSlider);

    // 操作按钮布局
    auto *actionsGroup = new QGroupBox(tr("Actions"), this);
    auto *actionsLayout = new QGridLayout(actionsGroup);
    actionsLayout->addWidget(increaseButton, 0, 0);
    actionsLayout->addWidget(decreaseButton, 0, 1);
    actionsLayout->addWidget(resetButton, 1, 0);
    actionsLayout->addWidget(animateToButton, 1, 1);

    // 快速主题布局
    auto *themeLayout = new QHBoxLayout(themeGroup);
    themeLayout->addWidget(classicThemeButton);
    themeLayout->addWidget(darkThemeButton);
    themeLayout->addWidget(modernThemeButton);
    themeLayout->addWidget(flatThemeButton);

    // 组装控制面板
    controlLayout->addWidget(radiusGroup);
    controlLayout->addWidget(colorGroup);
    controlLayout->addWidget(animationGroup);
    controlLayout->addWidget(actionsGroup);
    controlLayout->addWidget(themeGroup);

    // 左侧：进度条显示
    auto *displayLayout = new QVBoxLayout();

    // 创建多个进度条展示不同状态
    auto *progressBarsGroup = new QGroupBox(tr("Progress Bars"), this);
    auto *progressBarsLayout = new QVBoxLayout(progressBarsGroup);

    // 主进度条
    auto *mainProgressLabel = new QLabel(tr("Main Progress Bar:"), this);
    progressBarsLayout->addWidget(mainProgressLabel);
    progressBarsLayout->addWidget(progressBar);

    // 添加一些固定值的进度条作为示例
    auto *exampleProgressLabel = new QLabel(tr("Example Progress Bars:"), this);
    progressBarsLayout->addWidget(exampleProgressLabel);

    auto *exampleProgress1 = new ProgressBar(this);
    exampleProgress1->setValue(25);
    exampleProgress1->setShowPercent(true);
    progressBarsLayout->addWidget(exampleProgress1);

    auto *exampleProgress2 = new ProgressBar(this);
    exampleProgress2->setValue(50);
    exampleProgress2->setShowPercent(true);
    progressBarsLayout->addWidget(exampleProgress2);

    auto *exampleProgress3 = new ProgressBar(this);
    exampleProgress3->setValue(75);
    exampleProgress3->setShowPercent(true);
    progressBarsLayout->addWidget(exampleProgress3);

    displayLayout->addWidget(progressBarsGroup);
    displayLayout->addWidget(valueGroup);

    // 主布局
    mainLayout->addLayout(displayLayout, 2);
    mainLayout->addWidget(controlPanel);

    setCentralWidget(mainWidget);
    resize(800, 500);
    setWindowTitle(tr("Progress Bar Example"));

    // ========== 颜色设置部分 ==========

    // 统一的颜色按钮更新函数
    auto updateColorButton = [](QPushButton *button, const QColor &color) {
        auto colorName = color.name(QColor::HexArgb).toUpper();

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

    // 初始化颜色按钮
    updateColorButton(chunkColorButton, progressBar->chunkColor());
    updateColorButton(textColorButton, progressBar->textColor());
    updateColorButton(baseColorButton, progressBar->baseColor());
    updateColorButton(backgroundColorButton, progressBar->backgroundColor());

    // ========== 信号连接 ==========

    // 数值控制 - 防止循环
    connect(valueSlider, &QSlider::valueChanged, this, [progressBar, valueSpinBox](int value) {
        valueSpinBox->blockSignals(true);
        progressBar->setValue(value);
        valueSpinBox->setValue(value);
        valueSpinBox->blockSignals(false);
    });

    connect(valueSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            [progressBar, valueSlider](double value) {
                valueSlider->blockSignals(true);
                progressBar->setValue(value);
                valueSlider->setValue(static_cast<int>(value));
                valueSlider->blockSignals(false);
            });

    // 范围控制
    connect(minValueSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            progressBar,
            &ProgressBar::setMinValue);
    connect(maxValueSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            progressBar,
            &ProgressBar::setMaxValue);

    // 圆角控制
    connect(radiusSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            progressBar,
            &ProgressBar::setRadius);
    connect(autoRadiusCheck, &QCheckBox::toggled, progressBar, &ProgressBar::setAutoRadius);

    // 显示设置
    connect(showPercentCheck, &QCheckBox::toggled, progressBar, &ProgressBar::setShowPercent);

    // 颜色设置
    connect(chunkColorButton,
            &QPushButton::clicked,
            this,
            [this, progressBar, chunkColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(progressBar->chunkColor(),
                                                      this,
                                                      tr("Select Chunk Color"));
                if (color.isValid()) {
                    progressBar->setChunkColor(color);
                    updateColorButton(chunkColorButton, color);
                }
            });

    connect(textColorButton,
            &QPushButton::clicked,
            this,
            [this, progressBar, textColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(progressBar->textColor(),
                                                      this,
                                                      tr("Select Text Color"));
                if (color.isValid()) {
                    progressBar->setTextColor(color);
                    updateColorButton(textColorButton, color);
                }
            });

    connect(baseColorButton,
            &QPushButton::clicked,
            this,
            [this, progressBar, baseColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(progressBar->baseColor(),
                                                      this,
                                                      tr("Select Base Color"));
                if (color.isValid()) {
                    progressBar->setBaseColor(color);
                    updateColorButton(baseColorButton, color);
                }
            });

    connect(backgroundColorButton,
            &QPushButton::clicked,
            this,
            [this, progressBar, backgroundColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(progressBar->backgroundColor(),
                                                      this,
                                                      tr("Select Background Color"));
                if (color.isValid()) {
                    progressBar->setBackgroundColor(color);
                    updateColorButton(backgroundColorButton, color);
                }
            });

    // 动画设置
    connect(animationDurationSlider,
            &QSlider::valueChanged,
            this,
            [progressBar, durationLabel](int value) {
                progressBar->setAnimationDuration(value);
                durationLabel->setText(tr("Animation duration: %1ms").arg(value));
            });

    // 操作按钮
    connect(increaseButton, &QPushButton::clicked, this, [progressBar]() {
        progressBar->increaseValue(10.0);
    });

    connect(decreaseButton, &QPushButton::clicked, this, [progressBar]() {
        progressBar->decreaseValue(10.0);
    });

    connect(resetButton, &QPushButton::clicked, progressBar, &ProgressBar::reset);
    connect(animateToButton, &QPushButton::clicked, this, [progressBar]() {
        progressBar->setValueAnimated(75.0);
    });

    // 快速主题
    connect(classicThemeButton,
            &QPushButton::clicked,
            this,
            [progressBar,
             updateColorButton,
             chunkColorButton,
             textColorButton,
             baseColorButton,
             backgroundColorButton]() {
                // 设置经典主题颜色
                progressBar->setChunkColor(QColor(77, 161, 255));
                progressBar->setTextColor(QColor(64, 65, 66));
                progressBar->setBaseColor(QColor(179, 179, 179));
                progressBar->setBackgroundColor(Qt::transparent);

                // 更新按钮颜色
                updateColorButton(chunkColorButton, progressBar->chunkColor());
                updateColorButton(textColorButton, progressBar->textColor());
                updateColorButton(baseColorButton, progressBar->baseColor());
                updateColorButton(backgroundColorButton, progressBar->backgroundColor());
            });

    connect(darkThemeButton,
            &QPushButton::clicked,
            this,
            [progressBar,
             updateColorButton,
             chunkColorButton,
             textColorButton,
             baseColorButton,
             backgroundColorButton]() {
                // 设置暗色主题颜色
                progressBar->setChunkColor(QColor(97, 175, 239));
                progressBar->setTextColor(QColor(220, 220, 220));
                progressBar->setBaseColor(QColor(60, 64, 72));
                progressBar->setBackgroundColor(QColor(33, 37, 43));

                // 更新按钮颜色
                updateColorButton(chunkColorButton, progressBar->chunkColor());
                updateColorButton(textColorButton, progressBar->textColor());
                updateColorButton(baseColorButton, progressBar->baseColor());
                updateColorButton(backgroundColorButton, progressBar->backgroundColor());
            });

    connect(modernThemeButton,
            &QPushButton::clicked,
            this,
            [progressBar,
             updateColorButton,
             chunkColorButton,
             textColorButton,
             baseColorButton,
             backgroundColorButton]() {
                // 设置现代主题颜色
                progressBar->setChunkColor(QColor(220, 20, 60));
                progressBar->setTextColor(QColor(25, 25, 112));
                progressBar->setBaseColor(QColor(211, 211, 211));
                progressBar->setBackgroundColor(QColor(240, 248, 255));

                // 更新按钮颜色
                updateColorButton(chunkColorButton, progressBar->chunkColor());
                updateColorButton(textColorButton, progressBar->textColor());
                updateColorButton(baseColorButton, progressBar->baseColor());
                updateColorButton(backgroundColorButton, progressBar->backgroundColor());
            });

    connect(flatThemeButton,
            &QPushButton::clicked,
            this,
            [progressBar,
             updateColorButton,
             chunkColorButton,
             textColorButton,
             baseColorButton,
             backgroundColorButton,
             radiusSpinBox,
             autoRadiusCheck]() {
                // 设置扁平化主题颜色
                progressBar->setChunkColor(QColor(46, 204, 113));
                progressBar->setTextColor(QColor(255, 255, 255));
                progressBar->setBaseColor(QColor(189, 195, 199));
                progressBar->setBackgroundColor(QColor(52, 73, 94));
                progressBar->setRadius(0);

                // 更新按钮颜色
                updateColorButton(chunkColorButton, progressBar->chunkColor());
                updateColorButton(textColorButton, progressBar->textColor());
                updateColorButton(baseColorButton, progressBar->baseColor());
                updateColorButton(backgroundColorButton, progressBar->backgroundColor());

                // 更新圆角控件状态
                radiusSpinBox->setValue(0);
                autoRadiusCheck->setChecked(false);
            });

    // 进度条信号连接
    connect(progressBar, &ProgressBar::valueChanged, this, [valueSlider, valueSpinBox](double value) {
        valueSlider->blockSignals(true);
        valueSlider->setValue(static_cast<int>(value));
        valueSlider->blockSignals(false);

        valueSpinBox->blockSignals(true);
        valueSpinBox->setValue(value);
        valueSpinBox->blockSignals(false);
    });

    connect(progressBar, &ProgressBar::valueIncreased, this, [](double newValue) {
        qDebug() << "Value increased to:" << newValue;
    });

    connect(progressBar, &ProgressBar::valueDecreased, this, [](double newValue) {
        qDebug() << "Value decreased to:" << newValue;
    });

    connect(progressBar, &ProgressBar::valueReset, this, []() { qDebug() << "Value reset"; });

    connect(progressBar, &ProgressBar::animationStarted, this, [](double oldValue, double newValue) {
        qDebug() << "Animation started from" << oldValue << "to" << newValue;
    });

    connect(progressBar, &ProgressBar::animationFinished, this, [](double value) {
        qDebug() << "Animation finished at:" << value;
    });

    // 初始化状态
    progressBar->setValue(0);
}

MainWindow::~MainWindow() {}
