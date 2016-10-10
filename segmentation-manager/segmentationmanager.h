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
    void alignSlices(size_t masterSliceNumber, Point a, size_t width, size_t height, int maxDeltaX = INT_MAX, int maxDeltaY = INT_MAX);
    void cropSlices(size_t firstSlice, size_t lastSlice, Point a, size_t width, size_t height );
    void exportResult(std::string path);
    void exportResult(std::string path, size_t firstSlice, size_t lastSlice);
    void exportSlicesImages(std::string path);
    void exportSlicesImages(std::string path, size_t firstSlice, size_t lastSlice);
    std::vector<Slice>& getSlices();
    Slice* getSlice(size_t sliceNumber);
    cv::Mat apply(size_t sliceNumber);
    bool isEmpty();
    size_t size();

private:
    std::vector<Slice> slices;
};

#endif // SEGMENTATIONMANAGER_H
