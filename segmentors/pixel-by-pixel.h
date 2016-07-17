#ifndef PIXEL_BY_PIXEL_H
#define PIXEL_BY_PIXEL_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../seed.h"
#include "segmenter.h"

class PixelByPixel : public Segmenter
{
private:
    cv::Mat img;
    std::vector<Seed> seeds;
public:
    PixelByPixel(cv::Mat img, std::vector<Seed> seeds);
    cv::Mat Apply();
};

#endif // PIXEL_BY_PIXEL_H
