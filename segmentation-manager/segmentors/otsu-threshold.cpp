#include <iostream>

#include "otsu-threshold.h"

OtsuThreshold::OtsuThreshold(Slice slice)
{
    this->img = slice.getImg();
}

OtsuThreshold::~OtsuThreshold()
{

}

cv::Mat OtsuThreshold::Apply()
{
    cv::Mat res;
    double level = cv::threshold(this->img, res, 0, 1, CV_THRESH_BINARY | CV_THRESH_OTSU);
    std::cout << "Otsu Level: " << level << std::endl;

    return res;
}
