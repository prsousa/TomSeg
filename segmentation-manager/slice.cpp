#include "slice.h"

#include <opencv2/highgui/highgui.hpp>
#include <QDebug>

using namespace std;

Slice::Slice()
{
    this->minimumFeatureSize = 35;
}

Slice::Slice(string filename) : Slice()
{
    this->filename = filename;
    this->img = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
}

cv::Mat &Slice::getImg()
{
    return this->img;
}

void Slice::setSegmentationResult(cv::Mat &segmentationResult)
{
    this->segmentationResult = segmentationResult;
}

void Slice::resetSegmentationResult()
{
    this->segmentationResult = cv::Mat();
}

cv::Mat &Slice::getSegmentationResult()
{
    return this->segmentationResult;
}

string Slice::getFilename()
{
    return this->filename;
}

void Slice::setSeeds(const std::vector<Seed>& seeds)
{
    this->seeds.clear();

    for( size_t i = 0; i < seeds.size(); i++ ) {
        this->seeds.push_back( seeds[i] );
    }
}

std::vector<Seed> &Slice::getSeeds()
{
    return this->seeds;
}

void Slice::setMinimumFeatureSize(int minimumFeautureSize)
{
    this->minimumFeatureSize = std::max(minimumFeautureSize, 5);
}

int Slice::getMinimumFeatureSize()
{
    return this->minimumFeatureSize;
}

void Slice::removeSeed(int seedPos)
{
    this->seeds.erase( seeds.begin() + seedPos );
}

void Slice::crop(Point a, size_t width, size_t height)
{
    cv::Rect roi(a.x, a.y, width, height);
    img = img(roi);
}
