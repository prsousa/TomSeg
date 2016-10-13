#include "slice.h"

#include <opencv2/highgui/highgui.hpp>
#include <QDebug>

using namespace std;

Slice::Slice()
{

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

void Slice::addSeed(const Seed seed)
{
    this->seeds.push_back(seed);
}

int Slice::seedsNumber()
{
    return this->seeds.size();
}

std::vector<Seed> &Slice::getSeeds()
{
    return this->seeds;
}

void Slice::removeSeed(int seedPos)
{
    this->seeds.erase( seeds.begin() + seedPos );
}

void Slice::crop(Point a, size_t width, size_t height)
{
    cv::Rect roi(a.x, a.y, width, height);
    img = img(roi);

    // if already segmented
    if( !segmentationResult.empty() ) {
        segmentationResult = segmentationResult(roi);
    }
}
