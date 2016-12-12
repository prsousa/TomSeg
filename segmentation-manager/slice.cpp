#include "slice.h"

#include <opencv2/highgui/highgui.hpp>

using namespace std;

Slice::Slice()
{

}

Slice::Slice(string filename) : Slice()
{
    this->filename = filename;
    this->img = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);

    this->roiFromOriginal.x = this->roiFromOriginal.y = 0;
    this->roiFromOriginal.width = img.cols;
    this->roiFromOriginal.height = img.rows;
}

cv::Mat &Slice::getImg()
{
    return this->img;
}

void Slice::getHistogram(int histogram[])
{
    for(int i = 0; i < 256; i++) {
        histogram[i] = 0;
    }

    for(int y = 0; y < img.rows; y++) {
        for(int x = 0; x < img.cols; x++) {
            histogram[ img.at<uchar>(y, x) ]++;
        }
    }
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

void Slice::resetSeeds()
{
    this->seeds.clear();
}

int Slice::seedsNumber()
{
    return this->seeds.size();
}

std::vector<Seed> &Slice::getSeeds()
{
    return this->seeds;
}

bool Slice::containsSeedWithId(int seedId)
{
    for( Seed s : this->seeds ) {
        if( s.getId() == seedId ) {
            return true;
        }
    }

    return false;
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

    vector<Seed>::iterator it;
    for( it = seeds.begin(); it != seeds.end(); it++ ) {
        Seed &seed = *it;
        seed.a.x = min( img.cols, max( 0, seed.a.x - a.x));
        seed.a.y = min( img.rows, max( 0, seed.a.y - a.y));
        seed.b.x = min( img.cols, max( 0, seed.b.x - a.x));
        seed.b.y = min( img.rows, max( 0, seed.b.y - a.y));

        if( !seed.getNumberPixeis() ) {
            seeds.erase( it-- );
        }
    }

    roiFromOriginal.x += a.x;
    roiFromOriginal.y += a.y;
    roiFromOriginal.width = width;
    roiFromOriginal.height = height;
}

cv::Rect Slice::getRoiFromOriginal() const
{
    return roiFromOriginal;
}
