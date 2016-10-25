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
    void alignSlices();
    void alignSlices(size_t masterSliceNumber, Point a, size_t width, size_t height, int maxDeltaX, int maxDeltaY);
    void cropSlices(size_t firstSlice, size_t lastSlice, Point a, size_t width, size_t height );
    void resetResults();
    void resetSeeds();
    void resetSeeds(int sliceIndex);
    void resetSeeds(int start, int end);
    void resetSeedsFrom(int start);
    void exportResult(std::string path);
    void exportResult(std::string path, size_t firstSlice, size_t lastSlice);
    void exportSlicesImages(std::string path);
    void exportSlicesImages(std::string path, size_t firstSlice, size_t lastSlice);
    std::vector<Slice>& getSlices();
    Slice* getSlice(size_t sliceNumber);
    void propagateSeeds(size_t sliceNumber);
    cv::Mat segment();
    bool isEmpty();
    size_t size();

    void setXLen(float value);
    void setYLen(float value);
    void setZLen(float value);

    void setMinimumFeatureSize(int value);
    int getMinimumFeatureSize() const;

    int getMorphologicalSize() const;
    void setMorphologicalSize(int value);

private:
    std::vector<Slice> slices;
    int minimumFeatureSize;
    int morphologicalSize;
    float xLen;
    float yLen;
    float zLen;
};

#endif // SEGMENTATIONMANAGER_H
