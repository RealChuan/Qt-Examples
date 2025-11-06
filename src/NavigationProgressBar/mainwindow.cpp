#include "mainwindow.h"
#include "navigationprogressbar.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建导航进度条控件
    auto *progressBar = new NavigationProgressBar(this);

    // 设置初始步骤信息
    QStringList steps = {"Order Placed", "Payment Confirmed", "Processing", "Shipping", "Delivered"};
    progressBar->setMessageList(steps);
    progressBar->setStep(2); // 设置当前步骤为第3步

    // 创建步骤控制滑动条
    auto *stepSlider = new QSlider(Qt::Vertical, this);
    stepSlider->setRange(0, progressBar->maxStep());
    stepSlider->setValue(progressBar->step());
    auto *stepLabel = new QLabel(tr("Current Step: %1").arg(progressBar->step()), this);

    // 创建步骤导航按钮
    auto *prevButton = new QPushButton(tr("Previous Step"), this);
    auto *nextButton = new QPushButton(tr("Next Step"), this);
    auto *resetButton = new QPushButton(tr("Reset"), this);

    // 创建颜色控制
    auto *backgroundColorButton = new QPushButton(this);
    auto *currentColorButton = new QPushButton(this);
    auto *foregroundColorButton = new QPushButton(this);

    // 创建间距控制
    auto *spacingSlider = new QSlider(Qt::Horizontal, this);
    spacingSlider->setRange(0, 50);
    spacingSlider->setValue(progressBar->spacing());
    auto *spacingLabel = new QLabel(tr("Spacing: %1").arg(progressBar->spacing()), this);

    // 创建状态显示
    auto *statusLabel = new QLabel(tr("Ready"), this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::Box);

    // 创建自定义步骤信息编辑
    auto *messageEdit = new QLineEdit(this);
    messageEdit->setPlaceholderText(tr("Enter step description..."));
    auto *addMessageButton = new QPushButton(tr("Add Step"), this);
    auto *clearMessagesButton = new QPushButton(tr("Clear All"), this);

    // 布局设置
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    // 进度条和步骤控制布局
    auto *progressLayout = new QHBoxLayout();
    progressLayout->addWidget(progressBar, 1);
    progressLayout->addWidget(stepSlider);

    auto *stepControlsLayout = new QVBoxLayout();
    stepControlsLayout->addWidget(stepLabel);
    stepControlsLayout->addWidget(prevButton);
    stepControlsLayout->addWidget(nextButton);
    stepControlsLayout->addWidget(resetButton);
    progressLayout->addLayout(stepControlsLayout);

    // 颜色和字体控制布局
    auto *appearanceLayout = new QGridLayout();
    appearanceLayout->addWidget(new QLabel(tr("Background Color:"), this), 0, 0);
    appearanceLayout->addWidget(backgroundColorButton, 0, 1);
    appearanceLayout->addWidget(new QLabel(tr("Current Step Color:"), this), 1, 0);
    appearanceLayout->addWidget(currentColorButton, 1, 1);
    appearanceLayout->addWidget(new QLabel(tr("Text Color:"), this), 2, 0);
    appearanceLayout->addWidget(foregroundColorButton, 2, 1);
    appearanceLayout->addWidget(spacingLabel, 3, 0);
    appearanceLayout->addWidget(spacingSlider, 3, 1);

    // 步骤信息编辑布局
    auto *messageLayout = new QHBoxLayout();
    messageLayout->addWidget(messageEdit);
    messageLayout->addWidget(addMessageButton);
    messageLayout->addWidget(clearMessagesButton);

    // 主布局组装
    mainLayout->addLayout(progressLayout);
    mainLayout->addLayout(appearanceLayout);
    mainLayout->addLayout(messageLayout);
    mainLayout->addWidget(statusLabel);

    setCentralWidget(mainWidget);
    resize(800, 380);
    setWindowTitle(tr("Navigation Progress Bar Example"));

    // ========== 信号连接 ==========

    // 步骤滑动条控制
    connect(stepSlider, &QSlider::valueChanged, progressBar, [progressBar, stepLabel](int value) {
        progressBar->setStep(value);
        stepLabel->setText(tr("Current Step: %1").arg(value));
    });

    // 步骤导航按钮
    connect(prevButton, &QPushButton::clicked, progressBar, [progressBar, stepSlider]() {
        progressBar->previous();
        stepSlider->setValue(progressBar->step());
    });

    connect(nextButton, &QPushButton::clicked, progressBar, [progressBar, stepSlider]() {
        progressBar->next();
        stepSlider->setValue(progressBar->step());
    });

    connect(resetButton, &QPushButton::clicked, progressBar, [progressBar, stepSlider]() {
        progressBar->reset();
        stepSlider->setValue(progressBar->step());
    });

    // 更新颜色按钮显示
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
    updateColorButton(backgroundColorButton, progressBar->backgroundColor());
    updateColorButton(currentColorButton, progressBar->currentBackgroundColor());
    updateColorButton(foregroundColorButton, progressBar->foregroundColor());

    // 背景颜色选择
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

    // 当前步骤颜色选择
    connect(currentColorButton,
            &QPushButton::clicked,
            this,
            [this, progressBar, currentColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(progressBar->currentBackgroundColor(),
                                                      this,
                                                      tr("Select Current Step Color"));
                if (color.isValid()) {
                    progressBar->setCurrentBackgroundColor(color);
                    updateColorButton(currentColorButton, color);
                }
            });

    // 文字颜色选择
    connect(foregroundColorButton,
            &QPushButton::clicked,
            this,
            [this, progressBar, foregroundColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(progressBar->foregroundColor(),
                                                      this,
                                                      tr("Select Text Color"));
                if (color.isValid()) {
                    progressBar->setForegroundColor(color);
                    updateColorButton(foregroundColorButton, color);
                }
            });

    // 间距控制
    connect(spacingSlider,
            &QSlider::valueChanged,
            progressBar,
            [progressBar, spacingLabel](int value) {
                progressBar->setSpacing(value);
                spacingLabel->setText(tr("Spacing: %1").arg(value));
            });

    // 步骤信息编辑
    connect(addMessageButton, &QPushButton::clicked, this, [progressBar, messageEdit]() {
        QString text = messageEdit->text().trimmed();
        if (!text.isEmpty()) {
            QStringList messages = progressBar->messageList();
            messages.append(text);
            progressBar->setMessageList(messages);
            messageEdit->clear();
        }
    });

    connect(clearMessagesButton, &QPushButton::clicked, this, [progressBar]() {
        progressBar->setMessageList(QStringList());
    });

    // 状态更新
    connect(progressBar, &NavigationProgressBar::stepChanged, this, [statusLabel](int step) {
        statusLabel->setText(tr("Step changed to: %1").arg(step));
    });

    connect(progressBar, &NavigationProgressBar::progressCompleted, this, [statusLabel]() {
        statusLabel->setText(tr("All steps completed!"));
    });

    // 当步骤列表变化时更新滑动条范围
    auto updateSliderRange = [stepSlider, progressBar]() {
        stepSlider->setRange(0, progressBar->maxStep());
        stepSlider->setValue(progressBar->step());
    };

    // 监听步骤列表变化
    // 由于NavigationProgressBar没有提供步骤列表变化的信号，我们通过定时器间接监听
    auto *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, [progressBar, updateSliderRange]() {
        static int lastMaxStep = -1;
        if (progressBar->maxStep() != lastMaxStep) {
            lastMaxStep = progressBar->maxStep();
            updateSliderRange();
        }
    });
    updateTimer->start(100); // 每100ms检查一次

    // 初始更新
    updateSliderRange();
}

MainWindow::~MainWindow() {}
