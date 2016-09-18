#include "slice.h"

#include <opencv2/highgui/highgui.hpp>
#include <QDebug>

using namespace std;

Slice::Slice()
{

}

Slice::Slice(string filename)
{
    this->filename = filename;
    this->img = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
}

cv::Mat &Slice::getImg()
{
    return this->img;
}

string Slice::getFilename()
{
    return this->filename;
}

void Slice::setSeeds(const std::vector<SeedInfo>& seedsInfo)
{
    this->seeds.clear();

    for( size_t i = 0; i < seedsInfo.size(); i++ ) {
        SeedInfo seedInfo = seedsInfo[i];
        Point a(seedInfo.x, seedInfo.y);
        Point b(seedInfo.x + seedInfo.width, seedInfo.y + seedInfo.height);

        this->seeds.push_back( Seed(this->img, seedInfo.id, a, b) );
    }
}

std::vector<Seed> &Slice::getSeeds()
{
    return this->seeds;
}
