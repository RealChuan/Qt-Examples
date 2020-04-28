#include "bubblewidget.h"

#include <QtWidgets>

#define TRIANGLE_WIDTH 15       // 小三角的宽度
#define TRIANGLE_HEIGHT 10      // 小三角的高度
#define BORDER_RADIUS 5         // 窗口边角的弧度

struct Triangle{
    int width = TRIANGLE_WIDTH;
    int height = TRIANGLE_HEIGHT;
    BubbleWidget::Direction direct;
};

class BubbleWidgetPrivate{
public:
    BubbleWidgetPrivate(QWidget *parent) : owner(parent){
        shadowEffect = new QGraphicsDropShadowEffect(owner);
        shadowEffect->setOffset(0, 0);
        shadowEffect->setColor(Qt::gray);
        shadowEffect->setBlurRadius(SHADOW_WIDTH);
        centralWidget = new QWidget(owner);
    }
    QWidget *owner;
    QGraphicsDropShadowEffect* shadowEffect;
    QWidget *centralWidget;
    Triangle triangle;
};

BubbleWidget::BubbleWidget(QWidget *parent) : QWidget(parent)
  , d(new BubbleWidgetPrivate(this))
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setupUI();
    resize(200, 300);
}

BubbleWidget::~BubbleWidget()
{
    qDebug() << "~BubbleWidget";
    delete d;
}

void BubbleWidget::setCentralWidget(QWidget *widget)
{
    QHBoxLayout *layout = new QHBoxLayout(d->centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(widget);
}

void BubbleWidget::setTriangleInfo(int width, int height)
{
    d->triangle.width = width;
    d->triangle.height = height;
}

void BubbleWidget::setDerection(BubbleWidget::Direction dir)
{
    d->triangle.direct = dir;
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
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 200));
    // 小三角区域;
    QPainterPath drawPath;
    QPolygon trianglePolygon;
    switch (d->triangle.direct)
    {
    case Top:
        trianglePolygon << QPoint(width() / 2, SHADOW_WIDTH);
        trianglePolygon << QPoint(width() / 2 + d->triangle.width / 2,
                                  d->triangle.height + SHADOW_WIDTH);
        trianglePolygon << QPoint(width() / 2 - d->triangle.width / 2,
                                  d->triangle.height + SHADOW_WIDTH);
        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, d->triangle.height + SHADOW_WIDTH,
                                      width() - SHADOW_WIDTH * 2,
                                      height() - SHADOW_WIDTH * 2 - d->triangle.height),
                                BORDER_RADIUS, BORDER_RADIUS);
        break;
    case Left:
        trianglePolygon << QPoint(SHADOW_WIDTH, d->triangle.height + SHADOW_WIDTH + SHADOW_WIDTH);
        trianglePolygon << QPoint(SHADOW_WIDTH + d->triangle.width, SHADOW_WIDTH + SHADOW_WIDTH);
        trianglePolygon << QPoint(SHADOW_WIDTH + d->triangle.width,
                                  d->triangle.height + SHADOW_WIDTH + SHADOW_WIDTH);
        drawPath.addRoundedRect(QRect(SHADOW_WIDTH + d->triangle.width, SHADOW_WIDTH,
                                      width() - SHADOW_WIDTH * 2 - d->triangle.width,
                                      height() - SHADOW_WIDTH * 2), BORDER_RADIUS, BORDER_RADIUS);
        break;
    case Right:
        trianglePolygon << QPoint(width() - SHADOW_WIDTH - d->triangle.width,
                                  SHADOW_WIDTH + SHADOW_WIDTH);
        trianglePolygon << QPoint(width() - SHADOW_WIDTH - d->triangle.width,
                                  d->triangle.height + SHADOW_WIDTH + SHADOW_WIDTH);
        trianglePolygon << QPoint(width() - SHADOW_WIDTH,
                                  d->triangle.height + SHADOW_WIDTH + SHADOW_WIDTH);
        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, SHADOW_WIDTH,
                                      width() - SHADOW_WIDTH * 2 - d->triangle.width,
                                      height() - SHADOW_WIDTH * 2), BORDER_RADIUS, BORDER_RADIUS);

        break;
    case Bottom:
        trianglePolygon << QPoint(width() / 2, height() - SHADOW_WIDTH);
        trianglePolygon << QPoint(width() / 2 + d->triangle.width / 2,
                                  height() - SHADOW_WIDTH - d->triangle.height);
        trianglePolygon << QPoint(width() / 2 - d->triangle.width / 2,
                                  height() - SHADOW_WIDTH - d->triangle.height);
        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, SHADOW_WIDTH, width() - SHADOW_WIDTH * 2,
                                      height() - SHADOW_WIDTH * 2 - d->triangle.height),
                                BORDER_RADIUS, BORDER_RADIUS);
        break;
    }
    drawPath.addPolygon(trianglePolygon);
    painter.drawPath(drawPath);
}

void BubbleWidget::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    emit aboutToClose();
}

void BubbleWidget::setupUI()
{
    setGraphicsEffect(d->shadowEffect);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(SHADOW_WIDTH + 10,
                               SHADOW_WIDTH + TRIANGLE_HEIGHT + 20,
                               SHADOW_WIDTH + 10,
                               SHADOW_WIDTH + 10);
    layout->addWidget(d->centralWidget);
}
