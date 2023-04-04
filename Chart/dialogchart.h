#ifndef DIALOGCHART_H
#define DIALOGCHART_H

#include <QDialog>
#include <QtCharts>

class DialogChart : public QDialog
{
public:
    explicit DialogChart(QGraphicsScene *scene, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    QChartView *m_view;
    QGraphicsScene *m_scene;
};

#endif // DIALOGCHART_H
