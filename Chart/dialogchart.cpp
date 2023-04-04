#include "dialogchart.h"

#include <QHBoxLayout>

DialogChart::DialogChart(QGraphicsScene *scene, QWidget *parent)
    : QDialog(parent)
    , m_scene(scene)
{
    m_view = new QChartView;
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setScene(scene);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_view);
}

void DialogChart::resizeEvent(QResizeEvent *event)
{
    qreal sx = event->size().width() / m_scene->width();
    qreal sy = event->size().height() / m_scene->height();
    qreal min = qMin(sx, sy);
    m_view->scale(min, min);
    m_view->show();

    qDebug() << min;
    QDialog::resizeEvent(event);
}
