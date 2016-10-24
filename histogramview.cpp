#include "histogramview.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

HistogramView::HistogramView(QWidget *parent) : QChartView(parent)
{
    this->setRenderHint(QPainter::Antialiasing);
}

void HistogramView::setSlice(Slice *slice)
{
    this->slice = slice;
}

void HistogramView::update()
{
    int histogram[256];
    slice->getHistogram(histogram);

    QLineSeries *series = new QLineSeries();
    for( int i = 0; i < 256; i++) {
        *series << QPointF(i, histogram[i]);
    }

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);

    this->setChart(chart);
}
