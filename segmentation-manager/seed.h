#ifndef SEED_H
#define SEED_H

#include <vector>

#include <opencv2/core/core.hpp>

#include "region.h"
#include "point.h"

#define EMPTY 255

static cv::RNG rng(12345);

class Seed : public Region
{
public:
    int id;
    float average, relativeStdDev;
    cv::Scalar color;

    Seed();
    Seed(cv::Mat& img, Point a, Point b);
    Seed(cv::Mat& img, int id, Point a, Point b);

    void draw(cv::Mat img);
    Seed* getSimilarSeed(std::vector<Seed>&);
    Seed getMoreSimilarSeedByAvg(std::vector<Seed>&);
    Seed getMoreSimilarSeedByStdDev(std::vector<Seed>&);

    friend std::ostream& operator<<(std::ostream& os, const Seed& s);
};

#endif // SEED_H
