#include <iostream>

#include "seed.h"

using namespace std;

Seed::Seed() :
    Region()
{

}

Seed::Seed(cv::Mat& img, Point a, Point b) :
    Region(img, a, b)
{
    this->id = 0;
    this->active = true;

    this->average = this->getAverageIntensity();

    // if stdDev is lower than 1: relativeStdDev = 1; else: relativeStdDev = stdDev
    this->relativeStdDev = fmax( this->getStandardDeviation(this->average), 1.0f );
    this->c_r = rng.uniform(0, 255);
    this->c_g = rng.uniform(0,255);
    this->c_b = rng.uniform(0,255);
}

Seed::Seed(cv::Mat& img, int id, Point a, Point b) :
    Seed(img, a, b)
{
    this->id = id;
}

void Seed::draw(cv::Mat img) {
    cv::Point a( this->a.x, this->a.y );
    cv::Point b( this->b.x, this->b.y );
    cv::Scalar color(c_r, c_g, c_b);

    cv::rectangle( img, a, b, color, 3 );
}

std::ostream& operator<<(std::ostream& os, const Seed& s) {
    os << "Seed #" << s.id << "\t[(" << s.a.x << ", " << s.a.y << "), (" << s.b.x << ", " << s.b.y << ")]" << "\tμ: " << s.average << "\tσ: " << s.relativeStdDev;
    return os;
}

Seed* Seed::getSimilarSeed(vector<Seed>& seeds) {
    for( size_t i = 0; i < seeds.size(); i++ ) {
        Seed s = seeds[i];
        if( (this->average >= s.average - s.relativeStdDev) && (this->average <= s.average + s.relativeStdDev )) {
            // está dentro da média ± desvio padrão
            float relax = 0.05;
            if( (this->relativeStdDev >= s.relativeStdDev * (1-relax)) && (this->relativeStdDev <= s.relativeStdDev*(1+relax)) ) {
                return &seeds[i];
            }
        }
    }

    return NULL;
}

Seed Seed::getMoreSimilarSeedByAvg(std::vector<Seed>& seeds) {
    Seed res;
    int minorAvgDiff = INT_MAX;

    for( Seed s : seeds ) {
        int currentAvgDiff = std::abs( this->average - s.average );
        if( minorAvgDiff > currentAvgDiff ) {
            minorAvgDiff = currentAvgDiff;
            res = s;
        }
    }

    return res;
}

Seed Seed::getMoreSimilarSeedByStdDev(std::vector<Seed>&seeds) {
    Seed res;
    int minorStdDevDiff = INT_MAX;

    for( Seed s : seeds ) {
        int currentStdDevDiff = std::abs( this->relativeStdDev - s.relativeStdDev );
        if( minorStdDevDiff > currentStdDevDiff ) {
            minorStdDevDiff = currentStdDevDiff;
            res = s;
        }
    }

    return res;
}

void Seed::getDistances(cv::Mat& labels, size_t* dists, int nLabels) {
    Point centerOfMass = this->centerOfMass();

    for( int i = 0; i < nLabels; i++ ) {
        dists[i] = INT_MAX;
    }


    // VERY EXPENSIVE!
    for( int y = 0; y < labels.rows; y++ ) {
        for( int x = 0; x < labels.cols; x++ ) {
            uchar label = labels.at<uchar>(y, x);
            if( label != EMPTY ) {
                size_t curDist = centerOfMass.distance(Point(x, y));
                if( dists[label] > curDist ) {
                    dists[label] = curDist;
                }
            }
        }
    }
}

Seed* Seed::getBestGradedSeed(std::vector<Seed> &seeds, cv::Mat& labels, int* grade)
{
    size_t* dists = new size_t[seeds.size()];

    this->getDistances(labels, dists, seeds.size());

    for( Seed s : seeds ) {
        float avgDiff = std::abs( this->average - s.average );
        float stdDevDiff = std::abs( this->relativeStdDev - s.relativeStdDev );
        int distance = dists[s.id];
        float density = 0.0;

        cout << avgDiff << "\t" << stdDevDiff << "\t" << distance << "\t" << density << endl;
    }

    delete dists;
}
