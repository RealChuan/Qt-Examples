#include "linechart.h"
#include "normalchartdata.h"
#include "callout.h"
#include "dialogchart.h"

class LineChartPrivate{
public:
    LineChartPrivate(ChartView *parent)
        :owner(parent){
        lineSeries = new QLineSeries(owner);
        scatterSeries = new QScatterSeries(owner);
        scatterSeries->setMarkerSize(8);

        chart = new QChart;
        chart->setTitle(QObject::tr("Line Chart"));
        chart->addSeries(lineSeries);
        chart->addSeries(scatterSeries);
        chart->createDefaultAxes();
        chart->axes(Qt::Horizontal).first()->setRange(0, 100);
        chart->axes(Qt::Vertical).first()->setRange(0, 100);
        chart->setAcceptHoverEvents(true);

        xValue = new QGraphicsSimpleTextItem(chart);
        yValue = new QGraphicsSimpleTextItem(chart);

        xValue->setPos(chart->size().width() - 170, 10);
        yValue->setPos(chart->size().width() - 85, 10);

        xValue->setText(QObject::tr("X: "));
        yValue->setText(QObject::tr("Y: "));

        button = new QPushButton(QObject::tr("Eject"), owner);
        button->move(5, 5);
        button->setFlat(true);
    }
    ChartView *owner;
    QChart *chart;
    QLineSeries *lineSeries;
    QScatterSeries *scatterSeries;

    QGraphicsSimpleTextItem *xValue;
    QGraphicsSimpleTextItem *yValue;

    Callout *toolTip = nullptr;
    QList<Callout *> toolTipList;

    QPushButton *button;
};

LineChart::LineChart(QWidget *parent)
    :ChartView(parent)
    ,d(new LineChartPrivate(this))
{
    setupUI();
    buildConnect();
}

LineChart::~LineChart()
{
    delete d;
}

void LineChart::resizeEvent(QResizeEvent *event)
{
    //d->chart->resize(event->size());
    d->xValue->setPos(d->chart->size().width() - 170, 10);
    d->yValue->setPos(d->chart->size().width() - 85, 10);
    const auto callouts = d->toolTipList;
    for (Callout *callout : callouts)
        callout->updateGeometry();
    QChartView::resizeEvent(event);
}

void LineChart::mouseMoveEvent(QMouseEvent *event)
{
    d->xValue->setText(QObject::tr("X: %1").arg(d->chart->mapToValue(event->pos()).x()));
    d->yValue->setText(QObject::tr("Y: %1").arg(d->chart->mapToValue(event->pos()).x()));
    QChartView::mouseMoveEvent(event);
}

void LineChart::wheelEvent(QWheelEvent *event)
{
    int nd = event->angleDelta().rx() / 8;
    double scale =1.0 / pow(0.9, nd / 15.0f);
    d->chart->zoom(scale);
    update();
    QChartView::wheelEvent(event);
}

void LineChart::keepToolTip()
{
    d->toolTipList.append(d->toolTip);
    d->toolTip = new Callout(d->chart);
}

void LineChart::toolTip(QPointF point, bool state)
{
    if (d->toolTip == nullptr)
        d->toolTip = new Callout(d->chart);

    if (state) {
        d->toolTip->setText(QString(tr("X: %1 \nY: %2 ")).arg(point.x()).arg(point.y()));
        d->toolTip->setAnchor(point);
        d->toolTip->setZValue(11);
        d->toolTip->updateGeometry();
        d->toolTip->show();
    } else {
        d->toolTip->hide();
    }
}

void LineChart::showDialog()
{
    hide();
    DialogChart *dialog = new DialogChart(d->chart->scene(), this);
    dialog->resize(640, 480);
    dialog->exec();
    delete dialog;
    dialog = nullptr;
    setChart(d->chart);
    show();
}

void LineChart::setupUI()
{
    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing);
    setChart(d->chart);
    PointList pointList = generateRandomDataPoints(5, 101);
    d->lineSeries->replace(pointList);
    d->scatterSeries->replace(pointList);
}

void LineChart::buildConnect()
{
    connect(d->scatterSeries, &QScatterSeries::clicked, this, &LineChart::keepToolTip);
    connect(d->scatterSeries, &QScatterSeries::hovered, this, &LineChart::toolTip);

    connect(d->button, &QPushButton::clicked, this, &LineChart::showDialog);
}
