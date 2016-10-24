#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include "segmentation-manager/slice.h"

#include <QChartView>

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
    Slice* slice;
};

#endif // HISTOGRAMWIDGET_H
