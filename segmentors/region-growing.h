#ifndef REGION_GROWING_H
#define REGION_GROWING_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../seed.h"
#include "segmenter.h"

class RegionGrowing : public Segmenter
{
private:
    cv::Mat img;
    std::vector<Seed> seeds;
public:
    RegionGrowing(cv::Mat img, std::vector<Seed> seeds);
    cv::Mat Apply();
};

#endif // REGION_GROWING_H
