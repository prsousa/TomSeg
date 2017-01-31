#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include "segmentation-manager/slice.h"

#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

class HistogramView : public QtCharts::QChartView
{
    Q_OBJECT
public:
    explicit HistogramView(QWidget *parent = 0);
    void setSlice(Slice* slice);
    void update();

signals:

public slots:

private:
    QtCharts::QChart *chart;
    QtCharts::QSplineSeries *series;
    QtCharts::QValueAxis *axisX;
    QtCharts::QValueAxis *axisY;

    Slice* slice;
};

#endif // HISTOGRAMWIDGET_H
