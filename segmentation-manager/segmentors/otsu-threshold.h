#ifndef OTSUTHRESHOLD_H
#define OTSUTHRESHOLD_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../slice.h"

class OtsuThreshold
{
private:
    cv::Mat img;
public:
    OtsuThreshold(Slice slice);
    ~OtsuThreshold();
    cv::Mat Apply();
};

#endif // OTSUTHRESHOLD_H
