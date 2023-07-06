#include "mainwindow.h"

#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    resize(1000, 618);
}

MainWindow::~MainWindow()
{
    qDeleteAll(m_imageVector);
}

void MainWindow::setupUI()
{
    auto view = new GridView(this);

    m_imageVector.clear();
    auto colorNames = QColor::colorNames();
    for (const QString &colorName : qAsConst(colorNames)) {
        QImage image(WIDTH, WIDTH, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor(colorName));
        QRect rect = image.rect();
        rect.adjust(1, 1, -1, -1);
        double radius = (WIDTH - 2) / 2.0;
        painter.drawRoundedRect(rect, radius, radius);

        m_imageVector.append(new ImageInfo{image, colorName});
    }
    view->setImageVector(m_imageVector);

    setCentralWidget(view);
}
