#ifndef SEED_H
#define SEED_H

#include <opencv2/core/core.hpp>

#include "region.h"
#include "point.h"

static cv::RNG rng(12345);

class Seed : public Region
{
public:
    float average, stdDev;
    cv::Scalar color;

    Seed(cv::Mat& img, Point a, Point b): Region(img, a, b) {
        this->average = this->getAverageIntensity();
        this->stdDev = this->getStandardDeviation(this->average);
        color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );

//        std::cout << "Average: " << this->average << std::endl;
//        std::cout << "StdDev.: " << this->stdDev << std::endl;
    }

    void draw(cv::Mat img) {

        cv::Point a( this->a.x, this->a.y );
        cv::Point b( this->b.x, this->b.y );

        cv::rectangle(img, a, b, this->color, 3);
    }
};

#endif // SEED_H
