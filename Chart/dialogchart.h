#ifndef DIALOGCHART_H
#define DIALOGCHART_H

#include <QDialog>
#include <QtCharts>

class DialogChart : public QDialog
{
public:
    DialogChart(QGraphicsScene *scene, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    QChartView *view;
    QGraphicsScene *m_scene;
};

#endif // DIALOGCHART_H
