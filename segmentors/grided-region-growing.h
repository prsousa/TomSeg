#ifndef GRIDED_REGION_GROWING_H
#define GRIDED_REGION_GROWING_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../seed.h"
#include "segmenter.h"

class GridedRegionGrowing : public Segmenter
{
private:
    cv::Mat img;
    std::vector<Seed> seeds;
    std::vector<int> intervals;

    int minimumPhaseSize;
    int cellSize;
    cv::Mat compressedImgMean;
    cv::Mat compressedImgStdDev;
public:
    GridedRegionGrowing(cv::Mat img, std::vector<Seed> seeds, int minimumPhaseSize);
    cv::Mat Apply();
};

#endif // GRIDED_REGION_GROWING_H
