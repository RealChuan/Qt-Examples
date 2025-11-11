#include "mainwindow.h"
#include "dashboardwidget.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建仪表盘控件
    auto *dashboard = new DashBoardWidget(this);

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

    // 创建角度控制
    auto *startAngleLabel = new QLabel(tr("Start angle:"), this);
    auto *startAngleSpinBox = new QDoubleSpinBox(this);
    startAngleSpinBox->setRange(-360, 360);
    startAngleSpinBox->setValue(-50);
    startAngleSpinBox->setSuffix("°");

    auto *endAngleLabel = new QLabel(tr("End angle:"), this);
    auto *endAngleSpinBox = new QDoubleSpinBox(this);
    endAngleSpinBox->setRange(-360, 360);
    endAngleSpinBox->setValue(230);
    endAngleSpinBox->setSuffix("°");

    // 创建刻度控制
    auto *scaleMajorLabel = new QLabel(tr("Major scales:"), this);
    auto *scaleMajorSpinBox = new QSpinBox(this);
    scaleMajorSpinBox->setRange(1, 100);
    scaleMajorSpinBox->setValue(10);

    auto *scaleMinorLabel = new QLabel(tr("Minor scales:"), this);
    auto *scaleMinorSpinBox = new QSpinBox(this);
    scaleMinorSpinBox->setRange(1, 100);
    scaleMinorSpinBox->setValue(5);

    // 创建文本控制
    auto *titleLabel = new QLabel(tr("Title:"), this);
    auto *titleEdit = new QLineEdit("DashBoard", this);

    auto *unitLabel = new QLabel(tr("Unit:"), this);
    auto *unitEdit = new QLineEdit("unit", this);

    // 创建颜色选择控件 - 使用颜色预览按钮
    auto *arcColorButton = new QPushButton(this);
    auto *scaleColorButton = new QPushButton(this);
    auto *pointerColorButton = new QPushButton(this);
    auto *textColorButton = new QPushButton(this);
    auto *backgroundColorButton = new QPushButton(this);
    auto *valueColorButton = new QPushButton(this);
    auto *titleColorButton = new QPushButton(this);

    // 创建动画控制
    auto *animationDurationSlider = new QSlider(Qt::Horizontal, this);
    animationDurationSlider->setRange(0, 2000);
    animationDurationSlider->setValue(500);
    auto *durationLabel = new QLabel(tr("Animation duration: 500ms"), this);

    // 创建操作按钮
    auto *increaseButton = new QPushButton(tr("Increase +1"), this);
    auto *decreaseButton = new QPushButton(tr("Decrease -1"), this);
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

    valueControlLayout->addWidget(new QLabel(tr("Current value:"), this), 0, 0);
    valueControlLayout->addWidget(valueSlider, 0, 1);
    valueControlLayout->addWidget(valueSpinBox, 0, 2);

    valueControlLayout->addWidget(minValueLabel, 1, 0);
    valueControlLayout->addWidget(minValueSpinBox, 1, 1, 1, 2);

    valueControlLayout->addWidget(maxValueLabel, 2, 0);
    valueControlLayout->addWidget(maxValueSpinBox, 2, 1, 1, 2);

    // 角度和刻度布局
    auto *geometryGroup = new QGroupBox(tr("Geometry settings"), this);
    auto *geometryLayout = new QGridLayout(geometryGroup);

    geometryLayout->addWidget(startAngleLabel, 0, 0);
    geometryLayout->addWidget(startAngleSpinBox, 0, 1);
    geometryLayout->addWidget(endAngleLabel, 1, 0);
    geometryLayout->addWidget(endAngleSpinBox, 1, 1);

    geometryLayout->addWidget(scaleMajorLabel, 2, 0);
    geometryLayout->addWidget(scaleMajorSpinBox, 2, 1);
    geometryLayout->addWidget(scaleMinorLabel, 3, 0);
    geometryLayout->addWidget(scaleMinorSpinBox, 3, 1);

    // 文本设置布局
    auto *textGroup = new QGroupBox(tr("Text settings"), this);
    auto *textLayout = new QGridLayout(textGroup);

    textLayout->addWidget(titleLabel, 0, 0);
    textLayout->addWidget(titleEdit, 0, 1);
    textLayout->addWidget(unitLabel, 1, 0);
    textLayout->addWidget(unitEdit, 1, 1);

    // 颜色控制布局
    auto *colorGroup = new QGroupBox(tr("Color settings"), this);
    auto *colorLayout = new QGridLayout(colorGroup);

    colorLayout->addWidget(new QLabel(tr("Arc color:"), this), 0, 0);
    colorLayout->addWidget(arcColorButton, 0, 1);
    colorLayout->addWidget(new QLabel(tr("Scale color:"), this), 1, 0);
    colorLayout->addWidget(scaleColorButton, 1, 1);
    colorLayout->addWidget(new QLabel(tr("Pointer color:"), this), 2, 0);
    colorLayout->addWidget(pointerColorButton, 2, 1);

    colorLayout->addWidget(new QLabel(tr("Text color:"), this), 0, 2);
    colorLayout->addWidget(textColorButton, 0, 3);
    colorLayout->addWidget(new QLabel(tr("Background color:"), this), 1, 2);
    colorLayout->addWidget(backgroundColorButton, 1, 3);
    colorLayout->addWidget(new QLabel(tr("Value color:"), this), 2, 2);
    colorLayout->addWidget(valueColorButton, 2, 3);
    colorLayout->addWidget(new QLabel(tr("Title color:"), this), 3, 2);
    colorLayout->addWidget(titleColorButton, 3, 3);

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

    // 左侧：仪表盘显示
    auto *displayLayout = new QVBoxLayout();
    displayLayout->addWidget(dashboard);
    displayLayout->addWidget(valueGroup);
    displayLayout->addWidget(textGroup);

    // 主布局
    mainLayout->addLayout(displayLayout, 2);
    mainLayout->addWidget(controlPanel);

    setCentralWidget(mainWidget);
    resize(800, 500);
    setWindowTitle(tr("DashBoard Widget Example"));

    // ========== 颜色设置部分改造 ==========

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
    updateColorButton(arcColorButton, dashboard->arcColor());
    updateColorButton(scaleColorButton, dashboard->scaleColor());
    updateColorButton(pointerColorButton, dashboard->pointerColor());
    updateColorButton(textColorButton, dashboard->textColor());
    updateColorButton(backgroundColorButton, dashboard->backgroundColor());
    updateColorButton(valueColorButton, dashboard->valueColor());
    updateColorButton(titleColorButton, dashboard->titleColor());

    // ========== 信号连接 - 防止循环触发 ==========

    // 数值控制 - 防止循环
    connect(valueSlider, &QSlider::valueChanged, this, [dashboard, valueSpinBox](int value) {
        // 阻塞spinbox信号避免循环
        valueSpinBox->blockSignals(true);
        dashboard->setValue(value);
        valueSpinBox->setValue(value);
        valueSpinBox->blockSignals(false);
    });

    connect(valueSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            [dashboard, valueSlider](double value) {
                // 阻塞slider信号避免循环
                valueSlider->blockSignals(true);
                dashboard->setValue(value);
                valueSlider->setValue(static_cast<int>(value));
                valueSlider->blockSignals(false);
            });

    // 范围控制
    connect(minValueSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            dashboard,
            &DashBoardWidget::setMinValue);
    connect(maxValueSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            dashboard,
            &DashBoardWidget::setMaxValue);

    // 角度和刻度控制
    connect(startAngleSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            dashboard,
            &DashBoardWidget::setStartAngle);
    connect(endAngleSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            dashboard,
            &DashBoardWidget::setEndAngle);
    connect(scaleMajorSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            dashboard,
            &DashBoardWidget::setScaleMajor);
    connect(scaleMinorSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            dashboard,
            &DashBoardWidget::setScaleMinor);

    // 文本控制
    connect(titleEdit, &QLineEdit::textChanged, dashboard, &DashBoardWidget::setTitle);
    connect(unitEdit, &QLineEdit::textChanged, dashboard, &DashBoardWidget::setUnit);

    // 颜色设置
    connect(arcColorButton,
            &QPushButton::clicked,
            this,
            [this, dashboard, arcColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(dashboard->arcColor(),
                                                      this,
                                                      tr("Select Arc Color"));
                if (color.isValid()) {
                    dashboard->setArcColor(color);
                    updateColorButton(arcColorButton, color);
                }
            });

    connect(scaleColorButton,
            &QPushButton::clicked,
            this,
            [this, dashboard, scaleColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(dashboard->scaleColor(),
                                                      this,
                                                      tr("Select Scale Color"));
                if (color.isValid()) {
                    dashboard->setScaleColor(color);
                    updateColorButton(scaleColorButton, color);
                }
            });

    connect(pointerColorButton,
            &QPushButton::clicked,
            this,
            [this, dashboard, pointerColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(dashboard->pointerColor(),
                                                      this,
                                                      tr("Select Pointer Color"));
                if (color.isValid()) {
                    dashboard->setPointerColor(color);
                    updateColorButton(pointerColorButton, color);
                }
            });

    connect(textColorButton,
            &QPushButton::clicked,
            this,
            [this, dashboard, textColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(dashboard->textColor(),
                                                      this,
                                                      tr("Select Text Color"));
                if (color.isValid()) {
                    dashboard->setTextColor(color);
                    updateColorButton(textColorButton, color);
                }
            });

    connect(backgroundColorButton,
            &QPushButton::clicked,
            this,
            [this, dashboard, backgroundColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(dashboard->backgroundColor(),
                                                      this,
                                                      tr("Select Background Color"));
                if (color.isValid()) {
                    dashboard->setBackgroundColor(color);
                    updateColorButton(backgroundColorButton, color);
                }
            });

    connect(valueColorButton,
            &QPushButton::clicked,
            this,
            [this, dashboard, valueColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(dashboard->valueColor(),
                                                      this,
                                                      tr("Select Value Color"));
                if (color.isValid()) {
                    dashboard->setValueColor(color);
                    updateColorButton(valueColorButton, color);
                }
            });

    connect(titleColorButton,
            &QPushButton::clicked,
            this,
            [this, dashboard, titleColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(dashboard->titleColor(),
                                                      this,
                                                      tr("Select Title Color"));
                if (color.isValid()) {
                    dashboard->setTitleColor(color);
                    updateColorButton(titleColorButton, color);
                }
            });

    // 动画设置
    connect(animationDurationSlider,
            &QSlider::valueChanged,
            this,
            [dashboard, durationLabel](int value) {
                dashboard->setAnimationDuration(value);
                durationLabel->setText(tr("Animation duration: %1ms").arg(value));
            });

    // 操作按钮
    connect(increaseButton, &QPushButton::clicked, this, [dashboard]() {
        dashboard->increaseValue(1.0);
    });

    connect(decreaseButton, &QPushButton::clicked, this, [dashboard]() {
        dashboard->decreaseValue(1.0);
    });

    connect(resetButton, &QPushButton::clicked, dashboard, &DashBoardWidget::reset);
    connect(animateToButton, &QPushButton::clicked, this, [dashboard]() {
        dashboard->setValueAnimated(75.0);
    });

    // 快速主题
    connect(classicThemeButton,
            &QPushButton::clicked,
            this,
            [dashboard,
             updateColorButton,
             arcColorButton,
             scaleColorButton,
             pointerColorButton,
             textColorButton,
             backgroundColorButton,
             valueColorButton,
             titleColorButton]() {
                // 设置经典主题颜色
                dashboard->setArcColor(QColor(56, 61, 74));
                dashboard->setScaleColor(QColor(4, 168, 173));
                dashboard->setPointerColor(QColor(4, 181, 200));
                dashboard->setTextColor(QColor(144, 133, 116));
                dashboard->setBackgroundColor(Qt::transparent);
                dashboard->setValueColor(QColor(144, 133, 116));
                dashboard->setTitleColor(QColor(144, 133, 116));

                // 更新按钮颜色
                updateColorButton(arcColorButton, dashboard->arcColor());
                updateColorButton(scaleColorButton, dashboard->scaleColor());
                updateColorButton(pointerColorButton, dashboard->pointerColor());
                updateColorButton(textColorButton, dashboard->textColor());
                updateColorButton(backgroundColorButton, dashboard->backgroundColor());
                updateColorButton(valueColorButton, dashboard->valueColor());
                updateColorButton(titleColorButton, dashboard->titleColor());
            });

    connect(darkThemeButton,
            &QPushButton::clicked,
            this,
            [dashboard,
             updateColorButton,
             arcColorButton,
             scaleColorButton,
             pointerColorButton,
             textColorButton,
             backgroundColorButton,
             valueColorButton,
             titleColorButton]() {
                // 设置暗色主题颜色
                dashboard->setArcColor(QColor(40, 44, 52));
                dashboard->setScaleColor(QColor(97, 175, 239));
                dashboard->setPointerColor(QColor(198, 120, 221));
                dashboard->setTextColor(QColor(171, 178, 191));
                dashboard->setBackgroundColor(QColor(33, 37, 43));
                dashboard->setValueColor(QColor(152, 195, 121));
                dashboard->setTitleColor(QColor(224, 108, 117));

                // 更新按钮颜色
                updateColorButton(arcColorButton, dashboard->arcColor());
                updateColorButton(scaleColorButton, dashboard->scaleColor());
                updateColorButton(pointerColorButton, dashboard->pointerColor());
                updateColorButton(textColorButton, dashboard->textColor());
                updateColorButton(backgroundColorButton, dashboard->backgroundColor());
                updateColorButton(valueColorButton, dashboard->valueColor());
                updateColorButton(titleColorButton, dashboard->titleColor());
            });

    connect(modernThemeButton,
            &QPushButton::clicked,
            this,
            [dashboard,
             updateColorButton,
             arcColorButton,
             scaleColorButton,
             pointerColorButton,
             textColorButton,
             backgroundColorButton,
             valueColorButton,
             titleColorButton]() {
                // 设置现代主题颜色
                dashboard->setArcColor(QColor(70, 130, 180));
                dashboard->setScaleColor(QColor(100, 149, 237));
                dashboard->setPointerColor(QColor(220, 20, 60));
                dashboard->setTextColor(QColor(47, 79, 79));
                dashboard->setBackgroundColor(QColor(240, 248, 255));
                dashboard->setValueColor(QColor(25, 25, 112));
                dashboard->setTitleColor(QColor(65, 105, 225));

                // 更新按钮颜色
                updateColorButton(arcColorButton, dashboard->arcColor());
                updateColorButton(scaleColorButton, dashboard->scaleColor());
                updateColorButton(pointerColorButton, dashboard->pointerColor());
                updateColorButton(textColorButton, dashboard->textColor());
                updateColorButton(backgroundColorButton, dashboard->backgroundColor());
                updateColorButton(valueColorButton, dashboard->valueColor());
                updateColorButton(titleColorButton, dashboard->titleColor());
            });

    // 仪表盘信号连接 - 防止循环
    connect(dashboard,
            &DashBoardWidget::valueChanged,
            this,
            [valueSlider, valueSpinBox](double value) {
                valueSlider->blockSignals(true);
                valueSlider->setValue(static_cast<int>(value));
                valueSlider->blockSignals(false);

                valueSpinBox->blockSignals(true);
                valueSpinBox->setValue(value);
                valueSpinBox->blockSignals(false);
            });

    connect(dashboard, &DashBoardWidget::valueIncreased, this, [](double newValue) {
        qDebug() << "Value increased to:" << newValue;
    });

    connect(dashboard, &DashBoardWidget::valueDecreased, this, [](double newValue) {
        qDebug() << "Value decreased to:" << newValue;
    });

    connect(dashboard, &DashBoardWidget::valueReset, this, []() { qDebug() << "Value reset"; });

    connect(dashboard,
            &DashBoardWidget::animationStarted,
            this,
            [](double oldValue, double newValue) {
                qDebug() << "Animation started from" << oldValue << "to" << newValue;
            });

    connect(dashboard, &DashBoardWidget::animationFinished, this, [](double value) {
        qDebug() << "Animation finished at:" << value;
    });

    // 初始化状态
    dashboard->setValue(0);
}

MainWindow::~MainWindow() {}
