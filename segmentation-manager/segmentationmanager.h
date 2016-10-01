#ifndef SEGMENTATIONMANAGER_H
#define SEGMENTATIONMANAGER_H

#include "seed.h"
#include "slice.h"

#include <opencv2/core/core.hpp>

class SegmentationManager
{
public:
    SegmentationManager();
    void setSlices(std::vector<std::string>& filenames);
    void setSliceSeeds(size_t sliceNumber, const std::vector<Seed>& seeds);
    std::vector<Slice>& getSlices();
    Slice* getSlice(size_t sliceNumber);
    cv::Mat apply(size_t sliceNumber);
    bool isEmpty();
    size_t size();

private:
    std::vector<Slice> slices;
};

#endif // SEGMENTATIONMANAGER_H
