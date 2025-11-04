#include "mainwindow.hpp"
#include "carousel3dview.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建3D轮播视图
    auto *carouselView = new Carousel3DView(this);

    // 创建基本控制按钮
    auto *previousButton = new QPushButton("Previous", this);
    auto *nextButton = new QPushButton("Next", this);

    // 创建动画控制
    auto *animationDurationSlider = new QSlider(Qt::Horizontal, this);
    animationDurationSlider->setRange(200, 2000);
    animationDurationSlider->setValue(carouselView->animationDuration());
    auto *durationLabel = new QLabel(tr("Animation: %1ms").arg(carouselView->animationDuration()),
                                     this);

    // 创建自动轮播控制
    auto *autoRotationCheckbox = new QCheckBox(tr("Auto Rotation"), this);
    autoRotationCheckbox->setChecked(carouselView->isAutoRotationEnabled());

    auto *rotationIntervalSlider = new QSlider(Qt::Horizontal, this);
    rotationIntervalSlider->setRange(500, 5000);
    rotationIntervalSlider->setValue(carouselView->autoRotationInterval());
    auto *intervalLabel = new QLabel(tr("Interval: %1ms").arg(carouselView->autoRotationInterval()),
                                     this);

    // 创建自动轮播控制按钮
    auto *startRotationButton = new QPushButton("Start", this);
    auto *stopRotationButton = new QPushButton("Stop", this);
    auto *toggleRotationButton = new QPushButton("Toggle", this);

    // 创建状态显示
    auto *statusLabel = new QLabel(tr("3D Image Carousel Ready"), this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    statusLabel->setLineWidth(1);

    // 主布局
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    // 轮播视图区域
    mainLayout->addWidget(carouselView, 1); // 轮播视图占据主要空间

    // 控制按钮区域
    auto *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(previousButton);
    controlLayout->addStretch();
    controlLayout->addWidget(nextButton);

    // 设置区域
    auto *settingsLayout = new QGridLayout();
    settingsLayout->addWidget(durationLabel, 0, 0);
    settingsLayout->addWidget(animationDurationSlider, 0, 1);
    settingsLayout->addWidget(intervalLabel, 1, 0);
    settingsLayout->addWidget(rotationIntervalSlider, 1, 1);
    settingsLayout->addWidget(autoRotationCheckbox, 2, 0);

    // 自动轮播按钮区域
    auto *autoRotationLayout = new QHBoxLayout();
    autoRotationLayout->addWidget(startRotationButton);
    autoRotationLayout->addWidget(stopRotationButton);
    autoRotationLayout->addWidget(toggleRotationButton);

    settingsLayout->addLayout(autoRotationLayout, 2, 1);

    // 主布局组装
    mainLayout->addLayout(controlLayout);
    mainLayout->addLayout(settingsLayout);
    mainLayout->addWidget(statusLabel);

    setCentralWidget(mainWidget);
    resize(650, 500);
    setWindowTitle(tr("3D Image Carousel with Controls"));

    // 连接信号和槽

    // 基本控制
    connect(previousButton, &QPushButton::clicked, carouselView, &Carousel3DView::rotateToPrevious);
    connect(nextButton, &QPushButton::clicked, carouselView, &Carousel3DView::rotateToNext);

    // 动画持续时间控制
    connect(animationDurationSlider,
            &QSlider::valueChanged,
            carouselView,
            &Carousel3DView::setAnimationDuration);
    connect(animationDurationSlider, &QSlider::valueChanged, this, [durationLabel](int value) {
        durationLabel->setText(tr("Animation: %1ms").arg(value));
    });

    // 自动轮播间隔控制
    connect(rotationIntervalSlider,
            &QSlider::valueChanged,
            carouselView,
            &Carousel3DView::setAutoRotationInterval);
    connect(rotationIntervalSlider, &QSlider::valueChanged, this, [intervalLabel](int value) {
        intervalLabel->setText(tr("Interval: %1ms").arg(value));
    });

    // 自动轮播启用控制
    connect(autoRotationCheckbox,
            &QCheckBox::toggled,
            carouselView,
            &Carousel3DView::setAutoRotationEnabled);

    // 自动轮播按钮控制
    connect(startRotationButton,
            &QPushButton::clicked,
            carouselView,
            &Carousel3DView::startAutoRotation);
    connect(stopRotationButton,
            &QPushButton::clicked,
            carouselView,
            &Carousel3DView::stopAutoRotation);
    connect(toggleRotationButton,
            &QPushButton::clicked,
            carouselView,
            &Carousel3DView::toggleAutoRotation);

    // 状态更新
    connect(previousButton, &QPushButton::clicked, this, [statusLabel]() {
        statusLabel->setText(tr("Rotating to previous item..."));
    });

    connect(nextButton, &QPushButton::clicked, this, [statusLabel]() {
        statusLabel->setText(tr("Rotating to next item..."));
    });

    connect(startRotationButton, &QPushButton::clicked, this, [statusLabel]() {
        statusLabel->setText(tr("Auto rotation started"));
    });

    connect(stopRotationButton, &QPushButton::clicked, this, [statusLabel]() {
        statusLabel->setText(tr("Auto rotation stopped"));
    });

    connect(toggleRotationButton, &QPushButton::clicked, this, [statusLabel, carouselView]() {
        if (carouselView->isAutoRotationEnabled()) {
            statusLabel->setText(tr("Auto rotation toggled: ON"));
        } else {
            statusLabel->setText(tr("Auto rotation toggled: OFF"));
        }
    });

    // 定时器用于清除状态消息
    auto *clearStatusTimer = new QTimer(this);
    clearStatusTimer->setSingleShot(true);
    connect(clearStatusTimer, &QTimer::timeout, this, [statusLabel]() {
        statusLabel->setText(tr("3D Image Carousel Ready"));
    });

    // 连接所有操作到状态清除定时器
    auto startClearTimer = [clearStatusTimer]() {
        clearStatusTimer->start(1500); // 1.5秒后清除状态
    };

    connect(previousButton, &QPushButton::clicked, startClearTimer);
    connect(nextButton, &QPushButton::clicked, startClearTimer);
    connect(startRotationButton, &QPushButton::clicked, startClearTimer);
    connect(stopRotationButton, &QPushButton::clicked, startClearTimer);
    connect(toggleRotationButton, &QPushButton::clicked, startClearTimer);

    // 初始状态
    statusLabel->setText(tr("3D Image Carousel Ready - Use controls to customize behavior"));
    clearStatusTimer->start(3000);
}

MainWindow::~MainWindow() {}
