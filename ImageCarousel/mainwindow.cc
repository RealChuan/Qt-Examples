#include "mainwindow.hpp"
#include "imagecarouselwidget.hpp"

#include <QtWidgets>

class MainWindow::MainWindowPrivate
{
public:
    MainWindowPrivate(MainWindow *q)
        : q_ptr(q)
    {
        imageCarouselWidget = new ImageCarouselWidget(q);
    }

    MainWindow *q_ptr;

    ImageCarouselWidget *imageCarouselWidget;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new MainWindowPrivate(this))
{
    setupUI();
    resize(1000, 618);
}

MainWindow::~MainWindow() {}

void MainWindow::onAddImage()
{
    auto path = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation)
                    .value(0, QDir::homePath());
    auto images = QFileDialog::getOpenFileNames(this,
                                                tr("Open Image"),
                                                path,
                                                tr("Images (*.png *.jpg)"));
    if (images.isEmpty()) {
        return;
    }

    d_ptr->imageCarouselWidget->clearImages();
    d_ptr->imageCarouselWidget->addImages(images);
}

void MainWindow::setupUI()
{
    auto openImageBtn = new QPushButton(tr("Add Image"), this);
    connect(openImageBtn, &QPushButton::clicked, this, &MainWindow::onAddImage);
    auto startBtn = new QPushButton(tr("Start"), this);
    connect(startBtn,
            &QPushButton::clicked,
            d_ptr->imageCarouselWidget,
            &ImageCarouselWidget::start);
    auto stopBtn = new QPushButton(tr("Stop"), this);
    connect(stopBtn, &QPushButton::clicked, d_ptr->imageCarouselWidget, &ImageCarouselWidget::stop);

    auto btnLayout = new QHBoxLayout;
    btnLayout->addWidget(openImageBtn);
    btnLayout->addWidget(startBtn);
    btnLayout->addWidget(stopBtn);

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->addWidget(d_ptr->imageCarouselWidget);
    layout->addLayout(btnLayout);
    setCentralWidget(widget);
}
