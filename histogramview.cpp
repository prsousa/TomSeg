#include "histogramview.h"

#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>

QT_CHARTS_USE_NAMESPACE

HistogramView::HistogramView(QWidget *parent) : QChartView(parent)
{
    this->setRenderHint(QPainter::Antialiasing);

    chart = new QChart();
    series = new QLineSeries();

    chart->legend()->hide();


    this->setChart(chart);
}

void HistogramView::setSlice(Slice *slice)
{
    this->slice = slice;
}

void HistogramView::update()
{
    int histogram[256];
    slice->getHistogram(histogram);

    series->clear();
    for( int i = 0; i < 256; i++ ) {
        *series << QPointF(i, histogram[i]);
    }

    chart->addSeries(series);
}
