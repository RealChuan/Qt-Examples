#include "mainwindow.h"
#include "dashboardwidget.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建仪表盘控件
    auto *dashboard = new DashBoardWidget(this);

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

    // 创建颜色选择控件
    auto *arcColorLabel = new QLabel(tr("Arc color:"), this);
    auto *arcColorButton = new QPushButton(this);

    auto *scaleColorLabel = new QLabel(tr("Scale color:"), this);
    auto *scaleColorButton = new QPushButton(this);

    auto *pointerColorLabel = new QLabel(tr("Pointer color:"), this);
    auto *pointerColorButton = new QPushButton(this);

    auto *textColorLabel = new QLabel(tr("Text color:"), this);
    auto *textColorButton = new QPushButton(this);

    auto *backgroundColorLabel = new QLabel(tr("Background color:"), this);
    auto *backgroundColorButton = new QPushButton(this);

    auto *valueColorLabel = new QLabel(tr("Value color:"), this);
    auto *valueColorButton = new QPushButton(this);

    auto *titleColorLabel = new QLabel(tr("Title color:"), this);
    auto *titleColorButton = new QPushButton(this);

    // 创建动画控制
    auto *animationEnabledCheckbox = new QCheckBox(tr("Enable animation"), this);
    animationEnabledCheckbox->setChecked(true);

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

    valueControlLayout->addWidget(valueLabel, 0, 0);
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

    colorLayout->addWidget(arcColorLabel, 0, 0);
    colorLayout->addWidget(arcColorButton, 0, 1);
    colorLayout->addWidget(scaleColorLabel, 1, 0);
    colorLayout->addWidget(scaleColorButton, 1, 1);
    colorLayout->addWidget(pointerColorLabel, 2, 0);
    colorLayout->addWidget(pointerColorButton, 2, 1);

    colorLayout->addWidget(textColorLabel, 0, 2);
    colorLayout->addWidget(textColorButton, 0, 3);
    colorLayout->addWidget(backgroundColorLabel, 1, 2);
    colorLayout->addWidget(backgroundColorButton, 1, 3);
    colorLayout->addWidget(valueColorLabel, 2, 2);
    colorLayout->addWidget(valueColorButton, 2, 3);
    colorLayout->addWidget(titleColorLabel, 3, 2);
    colorLayout->addWidget(titleColorButton, 3, 3);

    // 动画控制布局
    auto *animationGroup = new QGroupBox(tr("Animation settings"), this);
    auto *animationLayout = new QVBoxLayout(animationGroup);
    animationLayout->addWidget(animationEnabledCheckbox);
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
    // controlLayout->addWidget(valueGroup);
    controlLayout->addWidget(geometryGroup);
    // controlLayout->addWidget(textGroup);
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
    resize(1000, 618);
    setWindowTitle(tr("DashBoard Widget Example"));

    // 初始化颜色按钮
    auto updateColorButton = [](QPushButton *button, const QColor &color) {
        button->setStyleSheet(
            QString("background-color: %1; border: 1px solid gray;").arg(color.name()));
    };

    // 设置初始颜色
    updateColorButton(arcColorButton, dashboard->arcColor());
    updateColorButton(scaleColorButton, dashboard->scaleColor());
    updateColorButton(pointerColorButton, dashboard->pointerColor());
    updateColorButton(textColorButton, dashboard->textColor());
    updateColorButton(backgroundColorButton, dashboard->backgroundColor());
    updateColorButton(valueColorButton, dashboard->valueColor());
    updateColorButton(titleColorButton, dashboard->titleColor());

    // 连接信号和槽

    // 数值控制
    connect(valueSlider, &QSlider::valueChanged, this, [dashboard, valueSpinBox](int value) {
        dashboard->setValue(value);
        valueSpinBox->setValue(value);
    });

    connect(valueSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            [dashboard, valueSlider](double value) {
                dashboard->setValue(value);
                valueSlider->setValue(static_cast<int>(value));
            });

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
        arcColorButton,
        [dashboard](const QColor &color) { dashboard->setArcColor(color); },
        [dashboard]() { return dashboard->arcColor(); });

    setupColorConnection(
        scaleColorButton,
        [dashboard](const QColor &color) { dashboard->setScaleColor(color); },
        [dashboard]() { return dashboard->scaleColor(); });

    setupColorConnection(
        pointerColorButton,
        [dashboard](const QColor &color) { dashboard->setPointerColor(color); },
        [dashboard]() { return dashboard->pointerColor(); });

    setupColorConnection(
        textColorButton,
        [dashboard](const QColor &color) { dashboard->setTextColor(color); },
        [dashboard]() { return dashboard->textColor(); });

    setupColorConnection(
        backgroundColorButton,
        [dashboard](const QColor &color) { dashboard->setBackgroundColor(color); },
        [dashboard]() { return dashboard->backgroundColor(); });

    setupColorConnection(
        valueColorButton,
        [dashboard](const QColor &color) { dashboard->setValueColor(color); },
        [dashboard]() { return dashboard->valueColor(); });

    setupColorConnection(
        titleColorButton,
        [dashboard](const QColor &color) { dashboard->setTitleColor(color); },
        [dashboard]() { return dashboard->titleColor(); });

    // 动画设置
    connect(animationEnabledCheckbox,
            &QCheckBox::toggled,
            dashboard,
            &DashBoardWidget::setAnimationEnabled);
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
                updateColorButton(arcColorButton, QColor(56, 61, 74));
                updateColorButton(scaleColorButton, QColor(4, 168, 173));
                updateColorButton(pointerColorButton, QColor(4, 181, 200));
                updateColorButton(textColorButton, QColor(144, 133, 116));
                updateColorButton(backgroundColorButton, Qt::transparent);
                updateColorButton(valueColorButton, QColor(144, 133, 116));
                updateColorButton(titleColorButton, QColor(144, 133, 116));
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
                updateColorButton(arcColorButton, QColor(40, 44, 52));
                updateColorButton(scaleColorButton, QColor(97, 175, 239));
                updateColorButton(pointerColorButton, QColor(198, 120, 221));
                updateColorButton(textColorButton, QColor(171, 178, 191));
                updateColorButton(backgroundColorButton, QColor(33, 37, 43));
                updateColorButton(valueColorButton, QColor(152, 195, 121));
                updateColorButton(titleColorButton, QColor(224, 108, 117));
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
                updateColorButton(arcColorButton, QColor(70, 130, 180));
                updateColorButton(scaleColorButton, QColor(100, 149, 237));
                updateColorButton(pointerColorButton, QColor(220, 20, 60));
                updateColorButton(textColorButton, QColor(47, 79, 79));
                updateColorButton(backgroundColorButton, QColor(240, 248, 255));
                updateColorButton(valueColorButton, QColor(25, 25, 112));
                updateColorButton(titleColorButton, QColor(65, 105, 225));
            });

    // 仪表盘信号连接
    connect(dashboard,
            &DashBoardWidget::valueChanged,
            this,
            [valueLabel, valueSlider, valueSpinBox](double value) {
                valueLabel->setText(tr("Current value: %1").arg(value, 0, 'f', 2));
                valueSlider->setValue(static_cast<int>(value));
                valueSpinBox->setValue(value);
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
    valueLabel->setText(tr("Current value: 0.00"));
}

MainWindow::~MainWindow() {}
