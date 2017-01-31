#include "histogramview.h"

QT_CHARTS_USE_NAMESPACE

HistogramView::HistogramView(QWidget *parent) : QChartView(parent)
{
    this->setRenderHint(QPainter::Antialiasing);

    chart = new QChart();
    chart->setBackgroundVisible(false);
    series = new QSplineSeries();

    chart->legend()->hide();
    chart->addSeries(series);
    chart->setMargins(QMargins());

    axisX = new QValueAxis;
    axisX->setTickCount(5);
    axisX->setLabelFormat("%i");
    QFont font;
    font.setPointSize(10);
    axisX->setLabelsFont( font );
    axisX->setMax(255);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY = new QValueAxis;
    axisY->setMax(100);
    axisY->hide();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

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

    float sum = 0.0;
    for( int i = 0; i < 256; i++ ) {
        sum += histogram[i];
    }

    const int step = 16;
    for( int i = 0; i < 256; i+=step ) {
        int histVal = 0;
        for( int a = 0; a < step; a++ ) {
            histVal += histogram[i+a];
        }

        *series << QPointF(i, histVal/sum * 100);
    }
    // chart->update();
}
