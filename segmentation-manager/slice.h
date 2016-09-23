#ifndef SLICE_H
#define SLICE_H

#include "seed.h"
#include "seedinfo.h"

#include <vector>

class Slice
{
public:
    Slice();
    Slice(std::string filename);
    cv::Mat &getImg();
    std::string getFilename();
    void setSeeds(const std::vector<SeedInfo> &seedsInfo );
    std::vector<Seed> &getSeeds();
    void setMinimumFeatureSize(int minimumFeautureSize);
    int getMinimumFeatureSize();

protected:
    cv::Mat img;
    std::string filename;
    std::vector<Seed> seeds;
    int minimumFeatureSize;
};

#endif // SLICE_H
