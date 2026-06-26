#include "mainwindow.hpp"
#include "gridmodel.hpp"
#include "gridview.hpp"

#include <QPainter>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // 创建网格视图
    auto *gridView = new GridView(this);

    // 创建简单的演示数据
    GridCellList cellList;

    // 生成简单的颜色单元格
    const auto colorNames = QColor::colorNames();

    for (qsizetype i = 0; i < colorNames.size(); ++i) {
        const QString colorName = colorNames.at(i);
        QImage image(GridModel::DEFAULT_GRID_CELL_SIZE - 2,
                     GridModel::DEFAULT_GRID_CELL_SIZE - 2,
                     QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor(colorName));
        painter.setPen(Qt::NoPen);

        // 绘制简单的圆角矩形
        const QRect rect = image.rect().adjusted(2, 2, -2, -2);
        const auto radius = rect.width() / 2;
        painter.drawRoundedRect(rect, radius, radius);

        // 添加编号标签
        cellList.append(GridCell{image, u"Cell %1"_s.arg(i + 1)});
    }

    // 设置数据到视图
    gridView->setCellList(cellList);

    // 设置为主窗口的中心部件
    setCentralWidget(gridView);

    resize(800, 500);
    setWindowTitle(u"Grid View Model Example"_s);
}
