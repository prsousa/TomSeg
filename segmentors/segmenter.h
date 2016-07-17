#ifndef SEGMENTER_H
#define SEGMENTER_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class Segmenter
{
public:
    Segmenter(){}
    ~Segmenter(){}
    virtual cv::Mat Apply() = 0;
};

#endif // SEGMENTER_H
