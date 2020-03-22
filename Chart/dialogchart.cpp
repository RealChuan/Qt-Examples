#include "dialogchart.h"

#include <QHBoxLayout>

DialogChart::DialogChart(QGraphicsScene *scene, QWidget *parent)
    :QDialog(parent)
    ,m_scene(scene)
{
    view = new QChartView;
    view->setRenderHint(QPainter::Antialiasing);
    view->setScene(scene);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(view);
}

void DialogChart::resizeEvent(QResizeEvent *event)
{
    qreal sx = event->size().width() / m_scene->width();
    qreal sy = event->size().height() / m_scene->height();
    qreal min = qMin(sx, sy);
    view->scale(min, min);
    view->show();

    qDebug() << min;
    QDialog::resizeEvent(event);
}
