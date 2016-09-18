#ifndef SEGMENTATIONMANAGER_H
#define SEGMENTATIONMANAGER_H

#include "../seedinfo.h"
#include "seed.h"
#include "slice.h"

#include <opencv2/core/core.hpp>

class SegmentationManager
{
public:
    SegmentationManager();
    void setSlices(std::vector<std::string>& filenames);
    void setSliceSeeds(size_t sliceNumber, const std::vector<SeedInfo>& seedsInfo);
    int* apply(size_t sliceNumber);

private:
    std::vector<Slice> slices;
};

#endif // SEGMENTATIONMANAGER_H
