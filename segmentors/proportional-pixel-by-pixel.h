#ifndef PROPORTIONAL_PIXEL_BY_PIXEL_H
#define PROPORTIONAL_PIXEL_BY_PIXEL_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../seed.h"
#include "segmenter.h"

class ProportionalPixelByPixel : public Segmenter
{
private:
    cv::Mat img;
    std::vector<Seed> seeds;
    std::vector<int> intervals;
public:
    ProportionalPixelByPixel(cv::Mat img, std::vector<Seed> seeds);
    cv::Mat Apply();
};

#endif // PROPORTIONAL_PIXEL_BY_PIXEL_H
