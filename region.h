#ifndef REGION_H
#define REGION_H

#include <opencv2/core/core.hpp>

#include "point.h"

class Region
{
public:
    Point a, b;
    cv::Mat* img;

    Region(cv::Mat& img, Point a, Point b) {
        assert(a.x >= 0 && a.x <= img.cols && a.y >= 0 && a.y <= img.rows);
        assert(b.x >= 0 && b.x <= img.cols && b.y >= 0 && b.y <= img.rows);
        assert(a.x < b.x && a.y < b.y);

//        std::cout << a.x << " " << a.y << std::endl;

        this->img = &img;
        this->a = a;
        this->b = b;
    }

    inline int getNumberPixeis() {
        return ( (b.x - a.x) * (b.y - a.y) );
    }

    float getAverageIntensity() {
        int accum = 0;

        for(int y = a.y; y < b.y; y++) {
            for(int x = a.x; x < b.x; x++) {
                accum += img->at<uchar>(y, x);
            }
        }

        return accum / this->getNumberPixeis();
    }

    float getStandardDeviation(float average) {
        int accum = 0;

        for(int y = a.y; y < b.y; y++) {
            for(int x = a.x; x < b.x; x++) {
                accum += pow(img->at<uchar>(y, x) - average, 2.0f);
            }
        }

        return sqrt( accum / this->getNumberPixeis() );
    }
};

#endif // REGION_H
