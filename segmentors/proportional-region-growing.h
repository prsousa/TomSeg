#ifndef PROPORTIONAL_REGION_GROWING_H
#define PROPORTIONAL_REGION_GROWING_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../seed.h"
#include "segmenter.h"

class ProportionalRegionGrowing : public Segmenter
{
private:
    cv::Mat img;
    std::vector<Seed> seeds;
    std::vector<int> intervals;
    Seed FindNextSeed(cv::Mat labels, int minSize);
public:
    ProportionalRegionGrowing(cv::Mat img, std::vector<Seed> seeds);
    cv::Mat Apply();
};

#endif // PROPORTIONAL_REGION_GROWING_H
