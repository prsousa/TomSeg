#ifndef SEGMENTERTHREAD_H
#define SEGMENTERTHREAD_H

#include <QThread>

#include "segmentation-manager/segmentationmanager.h"


class SegmenterThread : public QThread
{
    Q_OBJECT
public:
    SegmenterThread();
    SegmenterThread(SegmentationManager* segManager, int sliceIndex);

protected:
    void run();

private:
    SegmentationManager* segManager;
    int sliceIndex;
};

#endif // SEGMENTERTHREAD_H
