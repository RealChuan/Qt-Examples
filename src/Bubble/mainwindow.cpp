#include "mainwindow.h"
#include "bubblewrapperwidget.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建气泡包装控件
    auto *bubbleWrapper = new BubbleWrapperWidget(this);
    BubbleWidget *bubble = bubbleWrapper->bubbleWidget();

    // ========== 创建控件 ==========

    // 文本编辑框
    auto *textEdit = new QLineEdit(tr("Hello! This is a bubble widget example."), this);
    textEdit->setMinimumWidth(300);

    // 方向选择
    auto *directionCombo = new QComboBox(this);
    directionCombo->addItem(tr("Left"), static_cast<int>(BubbleWidget::Direction::Left));
    directionCombo->addItem(tr("Right"), static_cast<int>(BubbleWidget::Direction::Right));
    directionCombo->addItem(tr("Top"), static_cast<int>(BubbleWidget::Direction::Top));
    directionCombo->addItem(tr("Bottom"), static_cast<int>(BubbleWidget::Direction::Bottom));
    directionCombo->setCurrentIndex(3); // Bottom

    // 三角形大小控制
    auto *triangleWidthSpin = new QSpinBox(this);
    triangleWidthSpin->setRange(0, 50);
    triangleWidthSpin->setValue(10);
    triangleWidthSpin->setSuffix(tr(" px"));

    auto *triangleHeightSpin = new QSpinBox(this);
    triangleHeightSpin->setRange(0, 50);
    triangleHeightSpin->setValue(10);
    triangleHeightSpin->setSuffix(tr(" px"));

    // 字体大小控制
    auto *fontSizeSpin = new QSpinBox(this);
    fontSizeSpin->setRange(8, 30);
    fontSizeSpin->setValue(15);
    fontSizeSpin->setSuffix(tr(" px"));

    // 样式控制滑块
    auto *borderRadiusSlider = new QSlider(Qt::Horizontal, this);
    borderRadiusSlider->setRange(0, 30);
    borderRadiusSlider->setValue(5);

    auto *textMarginSlider = new QSlider(Qt::Horizontal, this);
    textMarginSlider->setRange(0, 30);
    textMarginSlider->setValue(8);

    // 颜色选择 - 使用颜色预览按钮（类似第一个代码的风格）
    auto *penColorButton = new QPushButton(this);
    auto *brushColorButton = new QPushButton(this);
    auto *textColorButton = new QPushButton(this);

    // 动画控制
    auto *animationCheckbox = new QCheckBox(tr("Enable Animation"), this);
    animationCheckbox->setChecked(true);

    auto *animationDurationSlider = new QSlider(Qt::Horizontal, this);
    animationDurationSlider->setRange(0, 2000);
    animationDurationSlider->setValue(300);

    // 控制按钮
    auto *showAtCursorButton = new QPushButton(tr("Show at Cursor"), this);
    auto *showAtWidgetButton = new QPushButton(tr("Show near This Button"), this);
    auto *hideButton = new QPushButton(tr("Hide Bubble"), this);
    auto *toggleButton = new QPushButton(tr("Toggle Bubble"), this);

    // 状态显示
    auto *statusLabel = new QLabel(tr("Ready"), this);
    statusLabel->setAlignment(Qt::AlignCenter);

    // ========== 创建标签用于显示数值 ==========

    auto *borderRadiusLabel = new QLabel(tr("5 px"), this);
    auto *textMarginLabel = new QLabel(tr("8 px"), this);
    auto *animationDurationLabel = new QLabel(tr("300 ms"), this);

    // ========== 布局设置 ==========

    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    // 状态标签
    mainLayout->addWidget(statusLabel);

    // 文本设置
    auto *textLayout = new QHBoxLayout();
    textLayout->addWidget(new QLabel(tr("Text:"), this));
    textLayout->addWidget(textEdit);

    // 方向设置
    auto *directionLayout = new QHBoxLayout();
    directionLayout->addWidget(new QLabel(tr("Direction:"), this));
    directionLayout->addWidget(directionCombo);
    directionLayout->addWidget(new QLabel(tr("Triangle Width:"), this));
    directionLayout->addWidget(triangleWidthSpin);
    directionLayout->addWidget(new QLabel(tr("Triangle Height:"), this));
    directionLayout->addWidget(triangleHeightSpin);

    // 字体设置
    auto *fontLayout = new QHBoxLayout();
    fontLayout->addWidget(new QLabel(tr("Font Size:"), this));
    fontLayout->addWidget(fontSizeSpin);
    fontLayout->addStretch();

    // 样式设置网格布局
    auto *styleLayout = new QGridLayout();
    styleLayout->addWidget(new QLabel(tr("Border Radius:"), this), 0, 0);
    styleLayout->addWidget(borderRadiusSlider, 0, 1);
    styleLayout->addWidget(borderRadiusLabel, 0, 2);

    styleLayout->addWidget(new QLabel(tr("Text Margin:"), this), 1, 0);
    styleLayout->addWidget(textMarginSlider, 1, 1);
    styleLayout->addWidget(textMarginLabel, 1, 2);

    styleLayout->addWidget(new QLabel(tr("Border Color:"), this), 2, 0);
    styleLayout->addWidget(penColorButton, 2, 1, 1, 2);

    styleLayout->addWidget(new QLabel(tr("Background Color:"), this), 3, 0);
    styleLayout->addWidget(brushColorButton, 3, 1, 1, 2);

    styleLayout->addWidget(new QLabel(tr("Text Color:"), this), 4, 0);
    styleLayout->addWidget(textColorButton, 4, 1, 1, 2);

    // 动画设置
    auto *animationLayout = new QHBoxLayout();
    animationLayout->addWidget(animationCheckbox);
    animationLayout->addWidget(new QLabel(tr("Animation Duration:"), this));
    animationLayout->addWidget(animationDurationSlider);
    animationLayout->addWidget(animationDurationLabel);

    // 控制按钮
    auto *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(showAtCursorButton);
    controlLayout->addWidget(showAtWidgetButton);
    controlLayout->addWidget(hideButton);
    controlLayout->addWidget(toggleButton);

    // 组装主布局
    mainLayout->addLayout(textLayout);
    mainLayout->addLayout(directionLayout);
    mainLayout->addLayout(fontLayout);
    mainLayout->addLayout(styleLayout);
    mainLayout->addLayout(animationLayout);
    mainLayout->addLayout(controlLayout);
    mainLayout->addStretch();

    setCentralWidget(mainWidget);
    resize(700, 430);
    setWindowTitle(tr("Bubble Widget Example"));

    // ========== 信号连接 ==========

    // 文本属性连接
    connect(textEdit, &QLineEdit::textChanged, bubble, &BubbleWidget::setText);

    connect(fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), [bubble](int size) {
        QFont font = bubble->textFont();
        font.setPixelSize(size);
        bubble->setTextFont(font);
    });

    // 方向控制连接
    connect(directionCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            [bubble, directionCombo]() {
                BubbleWidget::Direction direction = static_cast<BubbleWidget::Direction>(
                    directionCombo->currentData().toInt());
                bubble->setDirection(direction);
            });

    // 三角形大小连接
    auto updateTriangleSize = [bubble, triangleWidthSpin, triangleHeightSpin]() {
        bubble->setTriangleSize(QSize(triangleWidthSpin->value(), triangleHeightSpin->value()));
    };
    connect(triangleWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), updateTriangleSize);
    connect(triangleHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), updateTriangleSize);

    // 样式控制连接
    connect(borderRadiusSlider, &QSlider::valueChanged, [bubble, borderRadiusLabel](int value) {
        borderRadiusLabel->setText(QString("%1 px").arg(value));
        bubble->setBorderRadius(value);
    });

    connect(textMarginSlider, &QSlider::valueChanged, [bubble, textMarginLabel](int value) {
        textMarginLabel->setText(QString("%1 px").arg(value));
        bubble->setTextMargin(value);
    });

    // 颜色控制连接 - 改造为类似第一个代码的风格
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
    updateColorButton(penColorButton, bubble->pen().color());
    updateColorButton(brushColorButton, bubble->brush().color());
    updateColorButton(textColorButton, bubble->textColor());

    // 边框颜色选择
    connect(penColorButton,
            &QPushButton::clicked,
            this,
            [this, bubble, penColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(bubble->pen().color(),
                                                      this,
                                                      tr("Select Border Color"));
                if (color.isValid()) {
                    QPen pen = bubble->pen();
                    pen.setColor(color);
                    bubble->setPen(pen);
                    updateColorButton(penColorButton, color);
                }
            });

    // 背景颜色选择
    connect(brushColorButton,
            &QPushButton::clicked,
            this,
            [this, bubble, brushColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(bubble->brush().color(),
                                                      this,
                                                      tr("Select Background Color"));
                if (color.isValid()) {
                    bubble->setBrush(QBrush(color));
                    updateColorButton(brushColorButton, color);
                }
            });

    // 文本颜色选择
    connect(textColorButton,
            &QPushButton::clicked,
            this,
            [this, bubble, textColorButton, updateColorButton]() {
                QColor color = QColorDialog::getColor(bubble->textColor(),
                                                      this,
                                                      tr("Select Text Color"));
                if (color.isValid()) {
                    bubble->setTextColor(color);
                    updateColorButton(textColorButton, color);
                }
            });

    // 动画控制连接
    connect(animationDurationSlider,
            &QSlider::valueChanged,
            [bubbleWrapper, animationDurationLabel, animationCheckbox](int value) {
                animationDurationLabel->setText(QString("%1 ms").arg(value));
                if (animationCheckbox->isChecked()) {
                    bubbleWrapper->setAnimationDuration(value);
                }
            });

    connect(animationCheckbox,
            &QCheckBox::toggled,
            [bubbleWrapper, animationDurationSlider](bool checked) {
                bubbleWrapper->setAnimationDuration(checked ? animationDurationSlider->value() : 0);
            });

    // 显示控制连接
    auto getCurrentDirection = [directionCombo]() {
        return static_cast<BubbleWidget::Direction>(directionCombo->currentData().toInt());
    };

    connect(showAtCursorButton, &QPushButton::clicked, [bubbleWrapper, getCurrentDirection]() {
        bubbleWrapper->showAt(QCursor::pos(), getCurrentDirection());
    });

    connect(showAtWidgetButton,
            &QPushButton::clicked,
            [bubbleWrapper, showAtWidgetButton, getCurrentDirection]() {
                bubbleWrapper->showNearWidget(showAtWidgetButton, getCurrentDirection(), 10);
            });

    connect(hideButton, &QPushButton::clicked, bubbleWrapper, &BubbleWrapperWidget::hideAnimated);

    connect(toggleButton, &QPushButton::clicked, [bubbleWrapper, getCurrentDirection]() {
        if (bubbleWrapper->isVisible()) {
            bubbleWrapper->hideAnimated();
        } else {
            bubbleWrapper->showAt(QCursor::pos(), getCurrentDirection());
        }
    });

    // 气泡包装控件状态信号连接
    connect(bubbleWrapper, &BubbleWrapperWidget::aboutToShow, [statusLabel]() {
        statusLabel->setText(tr("Bubble about to show"));
    });

    connect(bubbleWrapper, &BubbleWrapperWidget::aboutToClose, [statusLabel]() {
        statusLabel->setText(tr("Bubble about to close"));
    });

    connect(bubbleWrapper, &BubbleWrapperWidget::animationStarted, [statusLabel]() {
        statusLabel->setText(tr("Bubble animation started"));
    });

    connect(bubbleWrapper, &BubbleWrapperWidget::animationFinished, [statusLabel]() {
        statusLabel->setText(tr("Bubble animation finished"));
    });

    connect(bubbleWrapper, &BubbleWrapperWidget::animationStopped, [statusLabel]() {
        statusLabel->setText(tr("Bubble animation stopped"));
    });

    // 初始属性设置
    bubble->setText(textEdit->text());
    bubbleWrapper->setAnimationDuration(animationDurationSlider->value());
}

MainWindow::~MainWindow() {}
