#ifndef SEED_H
#define SEED_H

#include <vector>

#include <opencv2/core/core.hpp>

#include "region.h"
#include "point.h"

#define EMPTY 255

class Seed : public Region
{
public:
    float average, relativeStdDev;
    bool active;

    Seed();
    Seed(cv::Mat& img, Point a, Point b);
    Seed(cv::Mat& img, int id, Point a, Point b);

    static void getColor(int id, uchar color[3]);

    void draw(cv::Mat img);
    int getId();
    void setId(int id);
    Seed* getSimilarSeed(std::vector<Seed>&);
    Seed getMoreSimilarSeedByAvg(std::vector<Seed>&);
    Seed getMoreSimilarSeedByStdDev(std::vector<Seed>&);
    Seed* getBestGradedSeed(std::vector<Seed>&, cv::Mat& labels, float* bestGrade);

    friend std::ostream& operator<<(std::ostream& os, const Seed& s);

private:
    int id;
    void getDistances(cv::Mat& labels, size_t* dists, int nLabels);
};

#endif // SEED_H
