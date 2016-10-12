#include "segmenterthread.h"

SegmenterThread::SegmenterThread()
{

}

SegmenterThread::SegmenterThread(SegmentationManager *segManager, int sliceIndex)
{
    this->segManager = segManager;
    this->sliceIndex = sliceIndex;
}

void SegmenterThread::run()
{
    segManager->segment();
}
