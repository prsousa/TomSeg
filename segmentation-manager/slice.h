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
    void resetSegmentationResult();
    cv::Mat& getSegmentationResult();
    std::string getFilename();
    void setSeeds(const std::vector<Seed> &seeds );
    void addSeed(const Seed seed);
    int seedsNumber();
    std::vector<Seed> &getSeeds();
    void removeSeed(int seedPos);
    void crop(Point a, size_t width, size_t height);

protected:
    cv::Mat img;
    cv::Mat segmentationResult;
    std::string filename;
    std::vector<Seed> seeds;
};

#endif // SLICE_H
