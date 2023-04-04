#include "linechart.h"
#include "normalchartdata.h"
#include "callout.h"
#include "dialogchart.h"

class LineChart::LineChartPrivate
{
public:
    explicit LineChartPrivate(ChartView *parent)
        : q_ptr(parent)
    {
        lineSeries = new QLineSeries(q_ptr);
        scatterSeries = new QScatterSeries(q_ptr);
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

        button = new QPushButton(QObject::tr("Eject"), q_ptr);
        button->move(5, 5);
        button->setFlat(true);
    }

    ChartView *q_ptr;

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
    : ChartView(parent)
    , d_ptr(new LineChartPrivate(this))
{
    setupUI();
    buildConnect();
}

LineChart::~LineChart() = default;

void LineChart::resizeEvent(QResizeEvent *event)
{
    //d->chart->resize(event->size());
    d_ptr->xValue->setPos(d_ptr->chart->size().width() - 170, 10);
    d_ptr->yValue->setPos(d_ptr->chart->size().width() - 85, 10);
    const auto callouts = d_ptr->toolTipList;
    for (Callout *callout : callouts) {
        callout->updateGeometry();
    }
    QChartView::resizeEvent(event);
}

void LineChart::mouseMoveEvent(QMouseEvent *event)
{
    d_ptr->xValue->setText(QObject::tr("X: %1").arg(d_ptr->chart->mapToValue(event->pos()).x()));
    d_ptr->yValue->setText(QObject::tr("Y: %1").arg(d_ptr->chart->mapToValue(event->pos()).x()));
    QChartView::mouseMoveEvent(event);
}

void LineChart::wheelEvent(QWheelEvent *event)
{
    int nd = event->angleDelta().rx() / 8;
    double scale =1.0 / pow(0.9, nd / 15.0f);
    d_ptr->chart->zoom(scale);
    update();
    QChartView::wheelEvent(event);
}

void LineChart::keepToolTip()
{
    d_ptr->toolTipList.append(d_ptr->toolTip);
    d_ptr->toolTip = new Callout(d_ptr->chart);
}

void LineChart::toolTip(QPointF point, bool state)
{
    if (d_ptr->toolTip == nullptr) {
        d_ptr->toolTip = new Callout(d_ptr->chart);
    }

    if (state) {
        d_ptr->toolTip->setText(QString(tr("X: %1 \nY: %2 ")).arg(point.x()).arg(point.y()));
        d_ptr->toolTip->setAnchor(point);
        d_ptr->toolTip->setZValue(11);
        d_ptr->toolTip->updateGeometry();
        d_ptr->toolTip->show();
    } else {
        d_ptr->toolTip->hide();
    }
}

void LineChart::showDialog()
{
    hide();
    DialogChart *dialog = new DialogChart(d_ptr->chart->scene(), this);
    dialog->resize(640, 480);
    dialog->exec();
    delete dialog;
    dialog = nullptr;
    setChart(d_ptr->chart);
    show();
}

void LineChart::setupUI()
{
    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing);
    setChart(d_ptr->chart);
    PointList pointList = generateRandomDataPoints(5, 101);
    d_ptr->lineSeries->replace(pointList);
    d_ptr->scatterSeries->replace(pointList);
}

void LineChart::buildConnect()
{
    connect(d_ptr->scatterSeries, &QScatterSeries::clicked, this, &LineChart::keepToolTip);
    connect(d_ptr->scatterSeries, &QScatterSeries::hovered, this, &LineChart::toolTip);

    connect(d_ptr->button, &QPushButton::clicked, this, &LineChart::showDialog);
}
