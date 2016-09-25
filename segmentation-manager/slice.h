#ifndef SLICE_H
#define SLICE_H

#include "seed.h"

#include <vector>

class Slice
{
public:
    Slice();
    Slice(std::string filename);
    cv::Mat &getImg();
    void setSegmentationResult(cv::Mat& segmentationResult);
    cv::Mat& getSegmentationResult();
    std::string getFilename();
    void setSeeds(const std::vector<Seed> &seeds );
    std::vector<Seed> &getSeeds();
    void setMinimumFeatureSize(int minimumFeautureSize);
    int getMinimumFeatureSize();

protected:
    cv::Mat img;
    cv::Mat segmentationResult;
    std::string filename;
    std::vector<Seed> seeds;
    int minimumFeatureSize;
};

#endif // SLICE_H
