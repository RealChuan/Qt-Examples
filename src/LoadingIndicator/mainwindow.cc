#include "mainwindow.hpp"
#include "loadingindicator.hpp"

#include <QtWidgets>

#include <cmath>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // === 控件创建 ===
    auto *styleComboBox = new QComboBox(this);
    styleComboBox->addItem(tr("Rotating Dots"),
                           QVariant::fromValue(LoadingIndicator::AnimationStyle::RotatingDots));
    styleComboBox->addItem(tr("Pulsing Circle"),
                           QVariant::fromValue(LoadingIndicator::AnimationStyle::PulsingCircle));
    styleComboBox->addItem(tr("Bouncing Bars"),
                           QVariant::fromValue(LoadingIndicator::AnimationStyle::BouncingBars));
    styleComboBox->addItem(tr("Custom GIF"),
                           QVariant::fromValue(LoadingIndicator::AnimationStyle::CustomMovie));

    auto *speedSlider = new QSlider(Qt::Horizontal, this);
    speedSlider->setRange(50, 500);
    speedSlider->setValue(100);
    auto *speedValueLabel = new QLabel(u"Animation speed: 100 ms"_s, this);

    auto *showTextCheckbox = new QCheckBox(tr("Show Text"), this);
    showTextCheckbox->setChecked(true);
    auto *textEdit = new QLineEdit(tr("Loading..."), this);
    textEdit->setPlaceholderText(tr("Enter loading text"));

    auto *autoHideCheckbox = new QCheckBox(tr("Auto Hide After:"), this);
    autoHideCheckbox->setChecked(true);
    auto *durationSpinBox = new QSpinBox(this);
    durationSpinBox->setRange(1, 60);
    durationSpinBox->setValue(5);
    durationSpinBox->setSuffix(tr(" seconds"));

    auto *dotColorButton = new QPushButton(this);
    auto *bgColorButton = new QPushButton(this);
    auto *textColorButton = new QPushButton(this);

    auto *showButton = new QPushButton(tr("Show"), this);
    auto *hideButton = new QPushButton(tr("Hide"), this);

    auto *classicThemeButton = new QPushButton(tr("Classic"), this);
    auto *darkThemeButton = new QPushButton(tr("Dark"), this);
    auto *modernThemeButton = new QPushButton(tr("Modern"), this);

    // === 测试区域 + 加载指示器 ===
    auto *testArea = new QLabel(tr("Test loading area"), this);
    testArea->setAlignment(Qt::AlignCenter);
    testArea->setFrameStyle(QFrame::StyledPanel);
    testArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *indicator = new LoadingIndicator(testArea);
    indicator->hideOverlay();

    // === 自动隐藏定时器 ===
    auto *autoHideTimer = new QTimer(this);
    autoHideTimer->setSingleShot(true);

    // === 颜色按钮更新函数 (与 CircularProgress/Battery/DashBoard/Clock 一致) ===
    auto updateColorButton = [](QPushButton *button, const QColor &color) {
        const QString colorName = color.name(QColor::HexArgb).toUpper();

        // WCAG 相对亮度计算
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

    // === 颜色按钮初始化 ===
    updateColorButton(dotColorButton, indicator->color());
    updateColorButton(bgColorButton, indicator->backgroundColor());
    updateColorButton(textColorButton, indicator->textColor());

    // === 布局组装 (与 CircularProgress 一致: 左侧展示 + 右侧控制面板) ===
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(mainWidget);

    // 左侧: 测试区域
    auto *displayLayout = new QVBoxLayout();
    displayLayout->addWidget(testArea, 1);

    // 右侧: 控制面板
    auto *controlPanel = new QWidget(this);
    auto *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setAlignment(Qt::AlignTop);

    // 动画设置组
    auto *animationGroup = new QGroupBox(tr("Animation settings"), this);
    auto *animationLayout = new QVBoxLayout(animationGroup);

    auto *styleRow = new QHBoxLayout();
    styleRow->addWidget(new QLabel(tr("Style:"), this));
    styleRow->addWidget(styleComboBox);
    animationLayout->addLayout(styleRow);

    animationLayout->addWidget(speedValueLabel);
    animationLayout->addWidget(speedSlider);

    controlLayout->addWidget(animationGroup);

    // 文本与自动隐藏组
    auto *textGroup = new QGroupBox(tr("Text settings"), this);
    auto *textGroupLayout = new QVBoxLayout(textGroup);

    auto *textRow = new QHBoxLayout();
    textRow->addWidget(showTextCheckbox);
    textRow->addWidget(textEdit);
    textGroupLayout->addLayout(textRow);

    auto *autoHideRow = new QHBoxLayout();
    autoHideRow->addWidget(autoHideCheckbox);
    autoHideRow->addWidget(durationSpinBox);
    autoHideRow->addStretch();
    textGroupLayout->addLayout(autoHideRow);

    controlLayout->addWidget(textGroup);

    // 颜色设置组
    auto *colorGroup = new QGroupBox(tr("Color settings"), this);
    auto *colorLayout = new QGridLayout(colorGroup);

    colorLayout->addWidget(new QLabel(tr("Animation:"), this), 0, 0);
    colorLayout->addWidget(dotColorButton, 0, 1);
    colorLayout->addWidget(new QLabel(tr("Background:"), this), 1, 0);
    colorLayout->addWidget(bgColorButton, 1, 1);
    colorLayout->addWidget(new QLabel(tr("Text:"), this), 2, 0);
    colorLayout->addWidget(textColorButton, 2, 1);

    controlLayout->addWidget(colorGroup);

    // 操作组
    auto *actionsGroup = new QGroupBox(tr("Actions"), this);
    auto *actionsLayout = new QGridLayout(actionsGroup);
    actionsLayout->addWidget(showButton, 0, 0);
    actionsLayout->addWidget(hideButton, 0, 1);

    controlLayout->addWidget(actionsGroup);

    // 快速主题组
    auto *themeGroup = new QGroupBox(tr("Quick themes"), this);
    auto *themeLayout = new QHBoxLayout(themeGroup);
    themeLayout->addWidget(classicThemeButton);
    themeLayout->addWidget(darkThemeButton);
    themeLayout->addWidget(modernThemeButton);

    controlLayout->addWidget(themeGroup);

    // 主布局
    mainLayout->addLayout(displayLayout, 2);
    mainLayout->addWidget(controlPanel);

    setCentralWidget(mainWidget);
    resize(800, 500);
    setWindowTitle(tr("Loading Indicator Example"));

    // === 信号槽连接 ===

    // 速度标签更新 + 指示器速度同步
    QObject::connect(
        speedSlider, &QSlider::valueChanged, this, [speedValueLabel, indicator](int value) {
            speedValueLabel->setText(tr("Animation speed: %1 ms").arg(value));
            indicator->setAnimationSpeed(value);
        });

    // 显示按钮
    QObject::connect(showButton, &QPushButton::clicked, this, [=]() {
        const auto style = styleComboBox->currentData().value<LoadingIndicator::AnimationStyle>();
        indicator->setAnimationStyle(style);

        if (style == LoadingIndicator::AnimationStyle::CustomMovie) {
            indicator->setMovie(u":/gif/resources/loading.gif"_s);
        }

        indicator->setAnimationSpeed(speedSlider->value());

        if (showTextCheckbox->isChecked() && !textEdit->text().isEmpty()) {
            indicator->setText(textEdit->text());
        } else {
            indicator->setText({});
        }

        indicator->showOverlay(testArea);

        if (autoHideCheckbox->isChecked()) {
            autoHideTimer->stop();
            autoHideTimer->start(durationSpinBox->value() * 1000);
        }
    });

    // 隐藏按钮
    QObject::connect(hideButton, &QPushButton::clicked, this, [indicator, autoHideTimer]() {
        indicator->hideOverlay();
        autoHideTimer->stop();
    });

    // 自动隐藏
    QObject::connect(
        autoHideTimer, &QTimer::timeout, this, [indicator]() { indicator->hideOverlay(); });

    // 样式切换
    QObject::connect(
        styleComboBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        [styleComboBox, indicator](int index) {
            const auto style
                = styleComboBox->itemData(index).value<LoadingIndicator::AnimationStyle>();
            indicator->setAnimationStyle(style);
            if (style == LoadingIndicator::AnimationStyle::CustomMovie) {
                indicator->setMovie(u":/gif/resources/loading.gif"_s);
            }
        });

    // 文本同步
    QObject::connect(textEdit,
                     &QLineEdit::textChanged,
                     this,
                     [showTextCheckbox, indicator](const QString &text) {
                         if (showTextCheckbox->isChecked()) {
                             indicator->setText(text);
                         }
                     });
    QObject::connect(
        showTextCheckbox, &QCheckBox::toggled, this, [textEdit, indicator](bool checked) {
            indicator->setText(checked && !textEdit->text().isEmpty() ? textEdit->text()
                                                                      : QString{});
        });

    // 颜色选择
    auto pickColor = [this](const QString &title, const QColor &initial) -> QColor {
        return QColorDialog::getColor(initial, this, title);
    };

    QObject::connect(dotColorButton, &QPushButton::clicked, this, [=]() {
        const QColor color = pickColor(tr("Select Animation Color"), indicator->color());
        if (color.isValid()) {
            indicator->setColor(color);
            updateColorButton(dotColorButton, color);
        }
    });

    QObject::connect(bgColorButton, &QPushButton::clicked, this, [=]() {
        const QColor color = pickColor(tr("Select Background Color"), indicator->backgroundColor());
        if (color.isValid()) {
            indicator->setBackgroundColor(color);
            updateColorButton(bgColorButton, color);
        }
    });

    QObject::connect(textColorButton, &QPushButton::clicked, this, [=]() {
        const QColor color = pickColor(tr("Select Text Color"), indicator->textColor());
        if (color.isValid()) {
            indicator->setTextColor(color);
            updateColorButton(textColorButton, color);
        }
    });

    // 指示器属性变化 → qDebug (与 CircularProgress 一致，不在 UI 显示)
    QObject::connect(indicator, &LoadingIndicator::textChanged, this, [](const QString &text) {
        qDebug() << "Text changed to:" << text;
    });
    QObject::connect(indicator, &LoadingIndicator::colorChanged, this, [](const QColor &color) {
        qDebug() << "Animation color changed:" << color;
    });
    QObject::connect(indicator, &LoadingIndicator::animationStyleChanged, this, [](auto style) {
        qDebug() << "Animation style changed:" << static_cast<int>(style);
    });

    // 快速主题 (颜色内联，与 CircularProgress/DashBoard/Clock 一致)
    auto applyTheme = [=](const QColor &animColor, const QColor &textColor, const QColor &bgColor) {
        indicator->setColor(animColor);
        indicator->setTextColor(textColor);
        indicator->setBackgroundColor(bgColor);
        updateColorButton(dotColorButton, animColor);
        updateColorButton(bgColorButton, bgColor);
        updateColorButton(textColorButton, textColor);
    };

    QObject::connect(classicThemeButton, &QPushButton::clicked, this, [applyTheme]() {
        applyTheme(QColor(0, 122, 255),         // #007aff systemBlue
                   QColor(28, 28, 30),          // #1c1c1e label
                   QColor(242, 242, 247, 230)); // #F2F2F7E6
    });
    QObject::connect(darkThemeButton, &QPushButton::clicked, this, [applyTheme]() {
        applyTheme(QColor(10, 132, 255),     // #0a84ff systemBlue dark
                   QColor(255, 255, 255),    // white
                   QColor(28, 28, 30, 230)); // #1c1c1eE6
    });
    QObject::connect(modernThemeButton, &QPushButton::clicked, this, [applyTheme]() {
        applyTheme(QColor(255, 149, 0),         // #ff9500 systemOrange
                   QColor(28, 28, 30),          // #1c1c1e
                   QColor(255, 255, 255, 230)); // #FFFFFFE6
    });
}

MainWindow::~MainWindow() = default;
