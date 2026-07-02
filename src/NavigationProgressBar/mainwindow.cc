#include "mainwindow.hpp"
#include "navigationprogressbar.hpp"

#include <QtWidgets>

#include <cmath>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // === 控件创建 ===
    auto *progressBar = new NavigationProgressBar(this);

    const QStringList steps = {
        u"Order Placed"_s, u"Payment Confirmed"_s, u"Processing"_s, u"Shipping"_s, u"Delivered"_s};
    progressBar->setMessageList(steps);
    progressBar->setStep(2);

    auto *stepSlider = new QSlider(Qt::Vertical, this);
    stepSlider->setRange(0, progressBar->maxStep());
    stepSlider->setValue(progressBar->step());
    auto *stepLabel = new QLabel(tr("Current Step: %1").arg(progressBar->step()), this);

    auto *prevButton = new QPushButton(tr("Previous"), this);
    auto *nextButton = new QPushButton(tr("Next"), this);
    auto *resetButton = new QPushButton(tr("Reset"), this);

    auto *backgroundColorButton = new QPushButton(this);
    auto *currentColorButton = new QPushButton(this);
    auto *foregroundColorButton = new QPushButton(this);

    auto *spacingSlider = new QSlider(Qt::Horizontal, this);
    spacingSlider->setRange(0, 50);
    spacingSlider->setValue(progressBar->spacing());
    auto *spacingLabel = new QLabel(tr("Spacing: %1").arg(progressBar->spacing()), this);

    auto *messageEdit = new QLineEdit(this);
    messageEdit->setPlaceholderText(tr("Enter step description..."));
    auto *addMessageButton = new QPushButton(tr("Add Step"), this);
    auto *clearMessagesButton = new QPushButton(tr("Clear All"), this);

    // === 颜色按钮更新函数 (与 CircularProgress 一致) ===
    auto updateColorButton = [](QPushButton *button, const QColor &color) {
        const QString colorName = color.name(QColor::HexArgb).toUpper();

        auto getRelativeLuminance = [](int r, int g, int b) {
            auto normalize = [](double x) {
                return x <= 0.03928 ? x / 12.92 : std::pow((x + 0.055) / 1.055, 2.4);
            };
            return 0.2126 * normalize(r / 255.0) + 0.7152 * normalize(g / 255.0)
                   + 0.0722 * normalize(b / 255.0);
        };

        const double luminance = getRelativeLuminance(color.red(), color.green(), color.blue());
        const QString textColor = luminance > 0.179 ? u"black"_s : u"white"_s;

        button->setStyleSheet(
            u"background-color: %1; color: %2; border: 1px solid gray; padding: 5px;"_s
                .arg(colorName)
                .arg(textColor));
        button->setText(colorName);
    };

    // 颜色按钮初始化
    updateColorButton(backgroundColorButton, progressBar->backgroundColor());
    updateColorButton(currentColorButton, progressBar->currentBackgroundColor());
    updateColorButton(foregroundColorButton, progressBar->foregroundColor());

    // === 布局组装 (横向组件 → 上下布局) ===
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    // 上方: 进度条 + 步骤滑动条
    auto *progressLayout = new QHBoxLayout();
    progressLayout->addWidget(progressBar, 1);
    progressLayout->addWidget(stepSlider);
    mainLayout->addLayout(progressLayout, 1);

    // 下方: 控制区域 (水平排列，利用宽度)
    auto *controlsLayout = new QHBoxLayout();

    // 步骤控制组
    auto *stepGroup = new QGroupBox(tr("Step controls"), this);
    auto *stepGroupLayout = new QHBoxLayout(stepGroup);
    stepGroupLayout->addWidget(stepLabel);
    stepGroupLayout->addWidget(prevButton);
    stepGroupLayout->addWidget(nextButton);
    stepGroupLayout->addWidget(resetButton);
    controlsLayout->addWidget(stepGroup);

    // 颜色设置组
    auto *colorGroup = new QGroupBox(tr("Color settings"), this);
    auto *colorLayout = new QGridLayout(colorGroup);
    colorLayout->addWidget(new QLabel(tr("Background:"), this), 0, 0);
    colorLayout->addWidget(backgroundColorButton, 0, 1);
    colorLayout->addWidget(new QLabel(tr("Current Step:"), this), 1, 0);
    colorLayout->addWidget(currentColorButton, 1, 1);
    colorLayout->addWidget(new QLabel(tr("Foreground:"), this), 2, 0);
    colorLayout->addWidget(foregroundColorButton, 2, 1);
    controlsLayout->addWidget(colorGroup);

    // 外观设置组
    auto *appearanceGroup = new QGroupBox(tr("Appearance"), this);
    auto *appearanceLayout = new QVBoxLayout(appearanceGroup);
    appearanceLayout->addWidget(spacingLabel);
    appearanceLayout->addWidget(spacingSlider);
    controlsLayout->addWidget(appearanceGroup);

    // 步骤管理组
    auto *messageGroup = new QGroupBox(tr("Step management"), this);
    auto *messageLayout = new QHBoxLayout(messageGroup);
    messageLayout->addWidget(messageEdit);
    messageLayout->addWidget(addMessageButton);
    messageLayout->addWidget(clearMessagesButton);
    controlsLayout->addWidget(messageGroup);

    mainLayout->addLayout(controlsLayout);

    setCentralWidget(mainWidget);
    resize(800, 380);
    setWindowTitle(tr("Navigation Progress Bar Example"));

    // === 信号槽连接 ===

    QObject::connect(stepSlider, &QSlider::valueChanged, this, [progressBar, stepLabel](int value) {
        progressBar->setStep(value);
        stepLabel->setText(tr("Current Step: %1").arg(value));
    });

    QObject::connect(prevButton, &QPushButton::clicked, this, [progressBar, stepSlider]() {
        progressBar->previous();
        stepSlider->setValue(progressBar->step());
    });

    QObject::connect(nextButton, &QPushButton::clicked, this, [progressBar, stepSlider]() {
        progressBar->next();
        stepSlider->setValue(progressBar->step());
    });

    QObject::connect(resetButton, &QPushButton::clicked, this, [progressBar, stepSlider]() {
        progressBar->reset();
        stepSlider->setValue(progressBar->step());
    });

    // 颜色选择
    auto pickColor = [this](const QString &title, const QColor &initial) -> QColor {
        return QColorDialog::getColor(initial, this, title);
    };

    QObject::connect(backgroundColorButton, &QPushButton::clicked, this, [=]() {
        const QColor color
            = pickColor(tr("Select Background Color"), progressBar->backgroundColor());
        if (color.isValid()) {
            progressBar->setBackgroundColor(color);
            updateColorButton(backgroundColorButton, color);
        }
    });

    QObject::connect(currentColorButton, &QPushButton::clicked, this, [=]() {
        const QColor color
            = pickColor(tr("Select Current Step Color"), progressBar->currentBackgroundColor());
        if (color.isValid()) {
            progressBar->setCurrentBackgroundColor(color);
            updateColorButton(currentColorButton, color);
        }
    });

    QObject::connect(foregroundColorButton, &QPushButton::clicked, this, [=]() {
        const QColor color
            = pickColor(tr("Select Foreground Color"), progressBar->foregroundColor());
        if (color.isValid()) {
            progressBar->setForegroundColor(color);
            updateColorButton(foregroundColorButton, color);
        }
    });

    QObject::connect(
        spacingSlider, &QSlider::valueChanged, this, [progressBar, spacingLabel](int value) {
            progressBar->setSpacing(value);
            spacingLabel->setText(tr("Spacing: %1").arg(value));
        });

    QObject::connect(addMessageButton, &QPushButton::clicked, this, [progressBar, messageEdit]() {
        const QString text = messageEdit->text().trimmed();
        if (!text.isEmpty()) {
            QStringList messages = progressBar->messageList();
            messages.append(text);
            progressBar->setMessageList(messages);
            messageEdit->clear();
        }
    });

    QObject::connect(clearMessagesButton, &QPushButton::clicked, this, [progressBar]() {
        progressBar->setMessageList({u"Step 1"_s});
    });

    QObject::connect(progressBar, &NavigationProgressBar::stepChanged, this, [](int step) {
        qDebug() << "Step changed to:" << step;
    });
    QObject::connect(progressBar, &NavigationProgressBar::progressCompleted, this, []() {
        qDebug() << "All steps completed!";
    });

    auto updateSliderRange = [stepSlider, progressBar]() {
        stepSlider->setRange(0, progressBar->maxStep());
        stepSlider->setValue(progressBar->step());
    };
    QObject::connect(progressBar,
                     &NavigationProgressBar::maxStepChanged,
                     this,
                     [updateSliderRange]() { updateSliderRange(); });
    updateSliderRange();
}

MainWindow::~MainWindow() = default;
