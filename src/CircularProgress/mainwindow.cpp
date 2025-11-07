#include "mainwindow.h"
#include "circularprogress.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建圆形进度控件
    auto *circularProgress = new CircularProgress("Loading Progress", this);

    // 创建数值控制
    auto *valueLabel = new QLabel(tr("Current value:"), this);
    valueLabel->setAlignment(Qt::AlignCenter);

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

    // 创建角度控制
    auto *startAngleLabel = new QLabel(tr("Start angle:"), this);
    auto *startAngleSpinBox = new QDoubleSpinBox(this);
    startAngleSpinBox->setRange(-360, 360);
    startAngleSpinBox->setValue(circularProgress->startAngle());
    startAngleSpinBox->setSuffix("°");

    auto *endAngleLabel = new QLabel(tr("End angle:"), this);
    auto *endAngleSpinBox = new QDoubleSpinBox(this);
    endAngleSpinBox->setRange(-360, 360);
    endAngleSpinBox->setValue(circularProgress->endAngle());
    endAngleSpinBox->setSuffix("°");

    // 创建显示设置
    auto *showPercentCheck = new QCheckBox(tr("Show percentage"), this);
    showPercentCheck->setChecked(true);

    // 创建文本控制
    auto *titleLabel = new QLabel(tr("Title:"), this);
    auto *titleEdit = new QLineEdit("Loading Progress", this);

    // 创建颜色选择控件
    auto *arcColorButton = new QPushButton(this);
    auto *textColorButton = new QPushButton(this);
    auto *titleColorButton = new QPushButton(this);
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

    // 布局设置
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(mainWidget);

    // 右侧：控制面板
    auto *controlPanel = new QWidget(this);
    auto *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setAlignment(Qt::AlignTop);

    // 数值控制布局
    auto *valueGroup = new QGroupBox(tr("Value settings"), this);
    auto *valueControlLayout = new QGridLayout(valueGroup);

    valueControlLayout->addWidget(valueLabel, 0, 0);
    valueControlLayout->addWidget(valueSlider, 0, 1);
    valueControlLayout->addWidget(valueSpinBox, 0, 2);

    valueControlLayout->addWidget(minValueLabel, 1, 0);
    valueControlLayout->addWidget(minValueSpinBox, 1, 1, 1, 2);

    valueControlLayout->addWidget(maxValueLabel, 2, 0);
    valueControlLayout->addWidget(maxValueSpinBox, 2, 1, 1, 2);

    valueControlLayout->addWidget(showPercentCheck, 3, 0, 1, 3);

    // 角度和显示布局
    auto *geometryGroup = new QGroupBox(tr("Geometry settings"), this);
    auto *geometryLayout = new QGridLayout(geometryGroup);

    geometryLayout->addWidget(startAngleLabel, 0, 0);
    geometryLayout->addWidget(startAngleSpinBox, 0, 1);
    geometryLayout->addWidget(endAngleLabel, 1, 0);
    geometryLayout->addWidget(endAngleSpinBox, 1, 1);

    // 文本设置布局
    auto *textGroup = new QGroupBox(tr("Text settings"), this);
    auto *textLayout = new QGridLayout(textGroup);

    textLayout->addWidget(titleLabel, 0, 0);
    textLayout->addWidget(titleEdit, 0, 1);

    // 颜色控制布局
    auto *colorGroup = new QGroupBox(tr("Color settings"), this);
    auto *colorLayout = new QGridLayout(colorGroup);

    colorLayout->addWidget(new QLabel(tr("Arc color:"), this), 0, 0);
    colorLayout->addWidget(arcColorButton, 0, 1);
    colorLayout->addWidget(new QLabel(tr("Text color:"), this), 1, 0);
    colorLayout->addWidget(textColorButton, 1, 1);
    colorLayout->addWidget(new QLabel(tr("Title color:"), this), 2, 0);
    colorLayout->addWidget(titleColorButton, 2, 1);

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

    // 组装控制面板
    controlLayout->addWidget(geometryGroup);
    controlLayout->addWidget(colorGroup);
    controlLayout->addWidget(animationGroup);
    controlLayout->addWidget(actionsGroup);
    controlLayout->addWidget(themeGroup);

    // 左侧：进度显示
    auto *displayLayout = new QVBoxLayout();
    displayLayout->addWidget(circularProgress);
    displayLayout->addWidget(valueGroup);
    displayLayout->addWidget(textGroup);

    // 主布局
    mainLayout->addLayout(displayLayout, 2);
    mainLayout->addWidget(controlPanel);

    setCentralWidget(mainWidget);
    resize(800, 500);
    setWindowTitle(tr("Circular Progress Example"));

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
    updateColorButton(arcColorButton, circularProgress->arcColor());
    updateColorButton(textColorButton, circularProgress->textColor());
    updateColorButton(titleColorButton, circularProgress->titleColor());
    updateColorButton(baseColorButton, circularProgress->baseColor());
    updateColorButton(backgroundColorButton, circularProgress->backgroundColor());

    // ========== 信号连接 ==========

    // 数值控制 - 防止循环
    connect(valueSlider, &QSlider::valueChanged, this, [circularProgress, valueSpinBox](int value) {
        valueSpinBox->blockSignals(true);
        circularProgress->setValue(value);
        valueSpinBox->setValue(value);
        valueSpinBox->blockSignals(false);
    });

    connect(valueSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            [circularProgress, valueSlider](double value) {
                valueSlider->blockSignals(true);
                circularProgress->setValue(value);
                valueSlider->setValue(static_cast<int>(value));
                valueSlider->blockSignals(false);
            });

    // 范围控制
    connect(minValueSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            circularProgress,
            &CircularProgress::setMinValue);
    connect(maxValueSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            circularProgress,
            &CircularProgress::setMaxValue);

    // 角度控制
    connect(startAngleSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            circularProgress,
            &CircularProgress::setStartAngle);
    connect(endAngleSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            circularProgress,
            &CircularProgress::setEndAngle);

    // 显示设置
    connect(showPercentCheck,
            &QCheckBox::toggled,
            circularProgress,
            &CircularProgress::setShowPercent);

    // 文本控制
    connect(titleEdit, &QLineEdit::textChanged, circularProgress, &CircularProgress::setTitle);

    // 颜色设置
    connect(arcColorButton,
            &QPushButton::clicked,
            this,
            [this, circularProgress, arcColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(circularProgress->arcColor(),
                                                      this,
                                                      tr("Select Arc Color"));
                if (color.isValid()) {
                    circularProgress->setArcColor(color);
                    updateColorButton(arcColorButton, color);
                }
            });

    connect(textColorButton,
            &QPushButton::clicked,
            this,
            [this, circularProgress, textColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(circularProgress->textColor(),
                                                      this,
                                                      tr("Select Text Color"));
                if (color.isValid()) {
                    circularProgress->setTextColor(color);
                    updateColorButton(textColorButton, color);
                }
            });

    connect(titleColorButton,
            &QPushButton::clicked,
            this,
            [this, circularProgress, titleColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(circularProgress->titleColor(),
                                                      this,
                                                      tr("Select Title Color"));
                if (color.isValid()) {
                    circularProgress->setTitleColor(color);
                    updateColorButton(titleColorButton, color);
                }
            });

    connect(baseColorButton,
            &QPushButton::clicked,
            this,
            [this, circularProgress, baseColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(circularProgress->baseColor(),
                                                      this,
                                                      tr("Select Base Color"));
                if (color.isValid()) {
                    circularProgress->setBaseColor(color);
                    updateColorButton(baseColorButton, color);
                }
            });

    connect(backgroundColorButton,
            &QPushButton::clicked,
            this,
            [this, circularProgress, backgroundColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(circularProgress->backgroundColor(),
                                                      this,
                                                      tr("Select Background Color"));
                if (color.isValid()) {
                    circularProgress->setBackgroundColor(color);
                    updateColorButton(backgroundColorButton, color);
                }
            });

    // 动画设置
    connect(animationDurationSlider,
            &QSlider::valueChanged,
            this,
            [circularProgress, durationLabel](int value) {
                circularProgress->setAnimationDuration(value);
                durationLabel->setText(tr("Animation duration: %1ms").arg(value));
            });

    // 操作按钮
    connect(increaseButton, &QPushButton::clicked, this, [circularProgress]() {
        circularProgress->increaseValue(10.0);
    });

    connect(decreaseButton, &QPushButton::clicked, this, [circularProgress]() {
        circularProgress->decreaseValue(10.0);
    });

    connect(resetButton, &QPushButton::clicked, circularProgress, &CircularProgress::reset);
    connect(animateToButton, &QPushButton::clicked, this, [circularProgress]() {
        circularProgress->setValueAnimated(75.0);
    });

    // 快速主题
    connect(classicThemeButton,
            &QPushButton::clicked,
            this,
            [circularProgress,
             updateColorButton,
             arcColorButton,
             textColorButton,
             titleColorButton,
             baseColorButton,
             backgroundColorButton]() {
                // 设置经典主题颜色
                circularProgress->setArcColor(QColor(77, 161, 255));
                circularProgress->setTextColor(QColor(77, 161, 255));
                circularProgress->setTitleColor(QColor(80, 80, 80));
                circularProgress->setBaseColor(QColor(179, 179, 179));
                circularProgress->setBackgroundColor(Qt::transparent);

                // 更新按钮颜色
                updateColorButton(arcColorButton, circularProgress->arcColor());
                updateColorButton(textColorButton, circularProgress->textColor());
                updateColorButton(titleColorButton, circularProgress->titleColor());
                updateColorButton(baseColorButton, circularProgress->baseColor());
                updateColorButton(backgroundColorButton, circularProgress->backgroundColor());
            });

    connect(darkThemeButton,
            &QPushButton::clicked,
            this,
            [circularProgress,
             updateColorButton,
             arcColorButton,
             textColorButton,
             titleColorButton,
             baseColorButton,
             backgroundColorButton]() {
                // 设置暗色主题颜色
                circularProgress->setArcColor(QColor(97, 175, 239));
                circularProgress->setTextColor(QColor(152, 195, 121));
                circularProgress->setTitleColor(QColor(224, 108, 117));
                circularProgress->setBaseColor(QColor(60, 64, 72));
                circularProgress->setBackgroundColor(QColor(33, 37, 43));

                // 更新按钮颜色
                updateColorButton(arcColorButton, circularProgress->arcColor());
                updateColorButton(textColorButton, circularProgress->textColor());
                updateColorButton(titleColorButton, circularProgress->titleColor());
                updateColorButton(baseColorButton, circularProgress->baseColor());
                updateColorButton(backgroundColorButton, circularProgress->backgroundColor());
            });

    connect(modernThemeButton,
            &QPushButton::clicked,
            this,
            [circularProgress,
             updateColorButton,
             arcColorButton,
             textColorButton,
             titleColorButton,
             baseColorButton,
             backgroundColorButton]() {
                // 设置现代主题颜色
                circularProgress->setArcColor(QColor(220, 20, 60));
                circularProgress->setTextColor(QColor(25, 25, 112));
                circularProgress->setTitleColor(QColor(65, 105, 225));
                circularProgress->setBaseColor(QColor(211, 211, 211));
                circularProgress->setBackgroundColor(QColor(240, 248, 255));

                // 更新按钮颜色
                updateColorButton(arcColorButton, circularProgress->arcColor());
                updateColorButton(textColorButton, circularProgress->textColor());
                updateColorButton(titleColorButton, circularProgress->titleColor());
                updateColorButton(baseColorButton, circularProgress->baseColor());
                updateColorButton(backgroundColorButton, circularProgress->backgroundColor());
            });

    // 进度控件信号连接
    connect(circularProgress,
            &CircularProgress::valueChanged,
            this,
            [valueLabel, valueSlider, valueSpinBox](double value) {
                valueLabel->setText(tr("Current value: %1").arg(value, 0, 'f', 2));

                valueSlider->blockSignals(true);
                valueSlider->setValue(static_cast<int>(value));
                valueSlider->blockSignals(false);

                valueSpinBox->blockSignals(true);
                valueSpinBox->setValue(value);
                valueSpinBox->blockSignals(false);
            });

    connect(circularProgress, &CircularProgress::valueIncreased, this, [](double newValue) {
        qDebug() << "Value increased to:" << newValue;
    });

    connect(circularProgress, &CircularProgress::valueDecreased, this, [](double newValue) {
        qDebug() << "Value decreased to:" << newValue;
    });

    connect(circularProgress, &CircularProgress::valueReset, this, []() {
        qDebug() << "Value reset";
    });

    connect(circularProgress,
            &CircularProgress::animationStarted,
            this,
            [](double oldValue, double newValue) {
                qDebug() << "Animation started from" << oldValue << "to" << newValue;
            });

    connect(circularProgress, &CircularProgress::animationFinished, this, [](double value) {
        qDebug() << "Animation finished at:" << value;
    });

    // 初始化状态
    circularProgress->setValue(0);
    valueLabel->setText(tr("Current value: 0.00"));
}

MainWindow::~MainWindow() {}
