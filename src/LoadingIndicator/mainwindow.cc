#include "mainwindow.hpp"
#include "loadingindicator.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建主控件和布局
    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(centralWidget);

    // 创建显示/隐藏按钮
    auto *showButton = new QPushButton(tr("Show Loading Indicator"), this);
    auto *hideButton = new QPushButton(tr("Hide Loading Indicator"), this);

    // 动画样式选择
    auto *styleLabel = new QLabel(tr("Animation Style:"), this);
    auto *styleComboBox = new QComboBox(this);
    styleComboBox->addItem(tr("Rotating Dots"),
                           QVariant::fromValue(LoadingIndicator::AnimationStyle::RotatingDots));
    styleComboBox->addItem(tr("Pulsing Circle"),
                           QVariant::fromValue(LoadingIndicator::AnimationStyle::PulsingCircle));
    styleComboBox->addItem(tr("Bouncing Bars"),
                           QVariant::fromValue(LoadingIndicator::AnimationStyle::BouncingBars));
    styleComboBox->addItem(tr("Custom GIF"),
                           QVariant::fromValue(LoadingIndicator::AnimationStyle::CustomMovie));

    // 动画速度控制
    auto *speedLabel = new QLabel(tr("Animation Speed:"), this);
    auto *speedSlider = new QSlider(Qt::Horizontal, this);
    speedSlider->setRange(50, 500);
    speedSlider->setValue(100);
    auto *speedValueLabel = new QLabel(tr("100 ms"), this);

    // 文本设置
    auto *showTextCheckbox = new QCheckBox(tr("Show Text"), this);
    showTextCheckbox->setChecked(true);
    auto *textEdit = new QLineEdit(this);
    textEdit->setText(tr("Loading..."));
    textEdit->setPlaceholderText(tr("Enter loading text"));

    // 自动隐藏设置
    auto *autoHideCheckbox = new QCheckBox(tr("Auto Hide After:"), this);
    autoHideCheckbox->setChecked(true);
    auto *durationSpinBox = new QSpinBox(this);
    durationSpinBox->setRange(1, 60);
    durationSpinBox->setValue(5);
    durationSpinBox->setSuffix(tr(" seconds"));

    // 颜色设置
    auto *dotColorLabel = new QLabel(tr("Animation Color:"), this);
    auto *dotColorButton = new QPushButton(this);

    auto *bgColorLabel = new QLabel(tr("Background Color:"), this);
    auto *bgColorButton = new QPushButton(this);

    auto *textColorLabel = new QLabel(tr("Text Color:"), this);
    auto *textColorButton = new QPushButton(this);

    // 状态显示
    auto *statusLabel = new QLabel(tr("Ready"), this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::Box);

    auto *loadingLabel = new QLabel(tr("Test loading"), this);
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setFrameStyle(QFrame::Box);
    loadingLabel->setMinimumSize(200, 200);
    loadingLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 布局设置
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(showButton);
    buttonLayout->addWidget(hideButton);

    auto *styleLayout = new QHBoxLayout();
    styleLayout->addWidget(styleLabel);
    styleLayout->addWidget(styleComboBox);

    auto *speedLayout = new QHBoxLayout();
    speedLayout->addWidget(speedLabel);
    speedLayout->addWidget(speedSlider);
    speedLayout->addWidget(speedValueLabel);

    auto *textLayout = new QHBoxLayout();
    textLayout->addWidget(showTextCheckbox);
    textLayout->addWidget(textEdit);

    auto *autoHideLayout = new QHBoxLayout();
    autoHideLayout->addWidget(autoHideCheckbox);
    autoHideLayout->addWidget(durationSpinBox);
    autoHideLayout->addStretch();

    // 颜色设置布局
    auto *colorLayout = new QGridLayout();
    colorLayout->addWidget(dotColorLabel, 0, 0);
    colorLayout->addWidget(dotColorButton, 0, 1);
    colorLayout->addWidget(bgColorLabel, 1, 0);
    colorLayout->addWidget(bgColorButton, 1, 1);
    colorLayout->addWidget(textColorLabel, 2, 0);
    colorLayout->addWidget(textColorButton, 2, 1);

    // 主布局
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(styleLayout);
    mainLayout->addLayout(speedLayout);
    mainLayout->addLayout(textLayout);
    mainLayout->addLayout(autoHideLayout);
    mainLayout->addLayout(colorLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(loadingLabel);

    setCentralWidget(centralWidget);

    // 创建加载指示器
    auto *indicator = new LoadingIndicator(loadingLabel);
    indicator->hideOverlay();
    connect(indicator, &LoadingIndicator::textChanged, this, [statusLabel](const QString &text) {
        statusLabel->setText(tr("Text changed to: %1").arg(text));
    });
    connect(indicator, &LoadingIndicator::textColorChanged, this, [statusLabel](const QColor &color) {
        statusLabel->setText(tr("Text color changed: %1").arg(color.name()));
    });
    connect(indicator, &LoadingIndicator::colorChanged, this, [statusLabel](const QColor &color) {
        statusLabel->setText(tr("Animation color changed: %1").arg(color.name()));
    });
    connect(indicator,
            &LoadingIndicator::backgroundColorChanged,
            this,
            [statusLabel](const QColor &color) {
                statusLabel->setText(tr("Background color changed: %1").arg(color.name()));
            });

    connect(indicator, &LoadingIndicator::animationSpeedChanged, this, [statusLabel](int ms) {
        statusLabel->setText(tr("Animation speed changed to %1 ms").arg(ms));
    });

    connect(indicator,
            &LoadingIndicator::animationStyleChanged,
            this,
            [statusLabel](LoadingIndicator::AnimationStyle style) {
                statusLabel->setText(
                    tr("Animation style changed: %1")
                        .arg(QMetaEnum::fromType<LoadingIndicator::AnimationStyle>().valueToKey(
                            static_cast<quint64>(style))));
            });

    // 创建自动隐藏定时器
    auto *autoHideTimer = new QTimer(this);
    autoHideTimer->setSingleShot(true);

    // 定义更新颜色按钮的函数
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
    updateColorButton(dotColorButton, indicator->color());
    updateColorButton(bgColorButton, indicator->backgroundColor());
    updateColorButton(textColorButton, indicator->textColor());

    // 更新速度标签
    connect(speedSlider, &QSlider::valueChanged, [speedValueLabel](int value) {
        speedValueLabel->setText(tr("%1 ms").arg(value));
    });

    // 显示加载指示器
    connect(showButton,
            &QPushButton::clicked,
            this,
            [loadingLabel,
             styleComboBox,
             speedSlider,
             showTextCheckbox,
             textEdit,
             autoHideCheckbox,
             durationSpinBox,
             statusLabel,
             indicator,
             autoHideTimer]() {
                // 设置动画样式
                LoadingIndicator::AnimationStyle style
                    = styleComboBox->currentData().value<LoadingIndicator::AnimationStyle>();
                indicator->setAnimationStyle(style);

                // 设置自定义GIF（如果选择了CustomMovie样式）
                if (style == LoadingIndicator::AnimationStyle::CustomMovie) {
                    indicator->setMovie(":/gif/resources/loading.gif");
                }

                // 设置动画速度
                indicator->setAnimationSpeed(speedSlider->value());

                // 设置文本
                if (showTextCheckbox->isChecked() && !textEdit->text().isEmpty()) {
                    indicator->setText(textEdit->text());
                } else {
                    indicator->setText("");
                }

                // 显示加载指示器
                indicator->showOverlay(loadingLabel);

                // 更新状态
                statusLabel->setText(tr("Loading indicator shown"));

                // 设置自动隐藏
                if (autoHideCheckbox->isChecked()) {
                    // 停止之前的定时器（如果有）
                    autoHideTimer->stop();
                    // 启动新的定时器
                    autoHideTimer->start(durationSpinBox->value() * 1000);
                }
            });

    // 隐藏加载指示器
    connect(hideButton, &QPushButton::clicked, this, [statusLabel, indicator, autoHideTimer]() {
        indicator->hideOverlay();
        autoHideTimer->stop(); // 停止自动隐藏定时器
        statusLabel->setText(tr("Loading indicator hidden"));
    });

    // 自动隐藏定时器超时
    connect(autoHideTimer, &QTimer::timeout, this, [statusLabel, indicator]() {
        indicator->hideOverlay();
        statusLabel->setText(tr("Loading indicator auto hidden"));
    });

    // 颜色选择
    connect(dotColorButton,
            &QPushButton::clicked,
            this,
            [this, dotColorButton, indicator, updateColorButton]() {
                QColor color = QColorDialog::getColor(indicator->color(),
                                                      this,
                                                      tr("Select Animation Color"));
                if (color.isValid()) {
                    indicator->setColor(color);
                    updateColorButton(dotColorButton, color);
                }
            });

    connect(bgColorButton,
            &QPushButton::clicked,
            this,
            [this, bgColorButton, indicator, updateColorButton]() {
                QColor color = QColorDialog::getColor(indicator->backgroundColor(),
                                                      this,
                                                      tr("Select Background Color"));
                if (color.isValid()) {
                    indicator->setBackgroundColor(color);
                    updateColorButton(bgColorButton, color);
                }
            });

    connect(textColorButton,
            &QPushButton::clicked,
            this,
            [this, textColorButton, indicator, updateColorButton]() {
                QColor color = QColorDialog::getColor(indicator->textColor(),
                                                      this,
                                                      tr("Select Text Color"));
                if (color.isValid()) {
                    indicator->setTextColor(color);
                    updateColorButton(textColorButton, color);
                }
            });

    // 样式变化时更新指示器
    connect(styleComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [styleComboBox, indicator](int index) {
                LoadingIndicator::AnimationStyle style
                    = styleComboBox->itemData(index).value<LoadingIndicator::AnimationStyle>();
                indicator->setAnimationStyle(style);

                if (style == LoadingIndicator::AnimationStyle::CustomMovie) {
                    indicator->setMovie(":/gif/resources/loading.gif");
                }
            });

    // 速度变化时更新指示器
    connect(speedSlider, &QSlider::valueChanged, this, [indicator](int value) {
        indicator->setAnimationSpeed(value);
    });

    // 文本变化时更新指示器
    connect(textEdit,
            &QLineEdit::textChanged,
            this,
            [showTextCheckbox, indicator](const QString &text) {
                if (showTextCheckbox->isChecked()) {
                    indicator->setText(text);
                }
            });

    // 显示文本复选框变化时更新指示器
    connect(showTextCheckbox, &QCheckBox::toggled, this, [textEdit, indicator](bool checked) {
        if (checked && !textEdit->text().isEmpty()) {
            indicator->setText(textEdit->text());
        } else {
            indicator->setText("");
        }
    });

    resize(450, 500);
    setWindowTitle(tr("Loading Indicator Example"));
}

MainWindow::~MainWindow() {}
