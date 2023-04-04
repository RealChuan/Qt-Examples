#include "bubblewidget.h"

#include <QtWidgets>

struct Triangle
{
    int width = 10;
    int height = 10;
    BubbleWidget::Direction direct = BubbleWidget::Direction::Top;
};

class BubbleWidget::BubbleWidgetPrivate
{
public:
    BubbleWidgetPrivate(QWidget *parent)
        : q_ptr(parent)
    {
        shadowEffect = new QGraphicsDropShadowEffect(q_ptr);
        shadowEffect->setOffset(0, 0);
        shadowEffect->setColor(Qt::gray);
        shadowEffect->setBlurRadius(shadowWidth);
        centralWidget = new QWidget(q_ptr);

        pen.setWidth(2);
    }

    QWidget *q_ptr;

    QGraphicsDropShadowEffect *shadowEffect;
    QWidget *centralWidget;
    Triangle triangle;

    qint64 shadowWidth = 10;
    qint64 borderRadius = 5;

    QPen pen = QColor(219, 186, 146);
    QBrush brush = QColor(255, 248, 240);

    QString text = QLatin1String("Are you OK!");
};

BubbleWidget::BubbleWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new BubbleWidgetPrivate(this))
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setupUI();
    resize(200, 100);
}

BubbleWidget::~BubbleWidget() {}

void BubbleWidget::setPen(const QPen &pen)
{
    d_ptr->pen = pen;
    update();
}

QPen BubbleWidget::pen() const
{
    return d_ptr->pen;
}

void BubbleWidget::setBrush(const QBrush &brush)
{
    d_ptr->brush = brush;
    update();
}

QBrush BubbleWidget::brush() const
{
    return d_ptr->brush;
}

void BubbleWidget::setShadowWidth(qint64 width)
{
    d_ptr->shadowWidth = width;
    update();
}

qint64 BubbleWidget::shadowWidth()
{
    return d_ptr->shadowWidth;
}

void BubbleWidget::setBorderRadius(qint64 radius)
{
    d_ptr->borderRadius = radius;
    update();
}

qint64 BubbleWidget::borderRadius()
{
    return d_ptr->borderRadius;
}

void BubbleWidget::setText(const QString &text)
{
    d_ptr->text = text;
    update();
}

QString BubbleWidget::text() const
{
    return d_ptr->text;
}

void BubbleWidget::setTriangleInfo(int width, int height)
{
    d_ptr->triangle.width = width;
    d_ptr->triangle.height = height;
    update();
}

void BubbleWidget::setDerection(BubbleWidget::Direction dir)
{
    d_ptr->triangle.direct = dir;
    update();
}

BubbleWidget::Direction BubbleWidget::direction()
{
    return d_ptr->triangle.direct;
}

void BubbleWidget::exec()
{
    show();
    QEventLoop loop;
    connect(this, &BubbleWidget::aboutToClose, &loop, &QEventLoop::quit);
    loop.exec();
}

void BubbleWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(d_ptr->pen);
    painter.setBrush(d_ptr->brush);

    // 小三角区域;
    QPainterPath drawPath;
    QPolygon trianglePolygon;
    QRect rect;
    switch (d_ptr->triangle.direct) {
    case Top:
        trianglePolygon << QPoint(width() / 2, d_ptr->shadowWidth);
        trianglePolygon << QPoint(width() / 2 + d_ptr->triangle.width / 2,
                                  d_ptr->triangle.height + d_ptr->shadowWidth);
        trianglePolygon << QPoint(width() / 2 - d_ptr->triangle.width / 2,
                                  d_ptr->triangle.height + d_ptr->shadowWidth);
        rect = QRect(d_ptr->shadowWidth,
                     d_ptr->triangle.height + d_ptr->shadowWidth,
                     width() - d_ptr->shadowWidth * 2,
                     height() - d_ptr->shadowWidth * 2 - d_ptr->triangle.height);
        drawPath.addRoundedRect(rect, d_ptr->borderRadius, d_ptr->borderRadius);
        break;
    case Left:
        trianglePolygon << QPoint(d_ptr->shadowWidth,
                                  d_ptr->triangle.height + d_ptr->shadowWidth + d_ptr->shadowWidth);
        trianglePolygon << QPoint(d_ptr->shadowWidth + d_ptr->triangle.width,
                                  d_ptr->shadowWidth + d_ptr->shadowWidth);
        trianglePolygon << QPoint(d_ptr->shadowWidth + d_ptr->triangle.width,
                                  d_ptr->triangle.height + d_ptr->shadowWidth + d_ptr->shadowWidth);
        rect = QRect(d_ptr->shadowWidth + d_ptr->triangle.width,
                     d_ptr->shadowWidth,
                     width() - d_ptr->shadowWidth * 2 - d_ptr->triangle.width,
                     height() - d_ptr->shadowWidth * 2);
        drawPath.addRoundedRect(rect, d_ptr->borderRadius, d_ptr->borderRadius);
        break;
    case Right:
        trianglePolygon << QPoint(width() - d_ptr->shadowWidth - d_ptr->triangle.width,
                                  d_ptr->shadowWidth + d_ptr->shadowWidth);
        trianglePolygon << QPoint(width() - d_ptr->shadowWidth - d_ptr->triangle.width,
                                  d_ptr->triangle.height + d_ptr->shadowWidth + d_ptr->shadowWidth);
        trianglePolygon << QPoint(width() - d_ptr->shadowWidth,
                                  d_ptr->triangle.height + d_ptr->shadowWidth + d_ptr->shadowWidth);
        rect = QRect(d_ptr->shadowWidth,
                     d_ptr->shadowWidth,
                     width() - d_ptr->shadowWidth * 2 - d_ptr->triangle.width,
                     height() - d_ptr->shadowWidth * 2);
        drawPath.addRoundedRect(rect, d_ptr->borderRadius, d_ptr->borderRadius);

        break;
    case Bottom:
        trianglePolygon << QPoint(width() / 2, height() - d_ptr->shadowWidth);
        trianglePolygon << QPoint(width() / 2 + d_ptr->triangle.width / 2,
                                  height() - d_ptr->shadowWidth - d_ptr->triangle.height);
        trianglePolygon << QPoint(width() / 2 - d_ptr->triangle.width / 2,
                                  height() - d_ptr->shadowWidth - d_ptr->triangle.height);
        rect = QRect(d_ptr->shadowWidth,
                     d_ptr->shadowWidth,
                     width() - d_ptr->shadowWidth * 2,
                     height() - d_ptr->shadowWidth * 2 - d_ptr->triangle.height);
        drawPath.addRoundedRect(rect, d_ptr->borderRadius, d_ptr->borderRadius);
        break;
    }
    drawPath.addPolygon(trianglePolygon);
    drawPath = drawPath.simplified();
    painter.drawPath(drawPath);

    painter.setFont(QFont("Microsoft YaHei", 11, 500));
    rect = rect.adjusted(5, 5, -5, -5);
    painter.drawText(rect, Qt::AlignCenter, d_ptr->text);
}

void BubbleWidget::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    emit aboutToClose();
}

void BubbleWidget::setupUI()
{
    setGraphicsEffect(d_ptr->shadowEffect);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(d_ptr->shadowWidth + 10,
                               d_ptr->shadowWidth + d_ptr->triangle.height + 20,
                               d_ptr->shadowWidth + 10,
                               d_ptr->shadowWidth + 10);
    layout->addWidget(d_ptr->centralWidget);
}
