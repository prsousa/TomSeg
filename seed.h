#ifndef SEED_H
#define SEED_H

#include <vector>

#include <opencv2/core/core.hpp>

#include "region.h"
#include "point.h"

#define EMPTY 255

static cv::RNG rng(12345);
static int nextId = 0;

class Seed : public Region
{
public:
    int id;
    float average, relativeStdDev;
    cv::Scalar color;

    Seed():Region() {}

    Seed(cv::Mat& img, Point a, Point b): Region(img, a, b) {
        this->id = nextId++;
        this->average = this->getAverageIntensity();

        // if stdDev is lower than 1: relativeStdDev = 1; else: relativeStdDev = stdDev
        this->relativeStdDev = fmax( this->getStandardDeviation(this->average), 1.0f );
        this->color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
    }

    void draw(cv::Mat img);
    Seed* getSimilarSeed(std::vector<Seed>&);

    friend std::ostream& operator<<(std::ostream& os, const Seed& s);
};

#endif // SEED_H
