#include "seedpropagater.h"

SeedPropagater::SeedPropagater()
{

}

SeedPropagater::SeedPropagater(std::vector<Slice>::iterator firstSlice, std::vector<Slice>::iterator lastSlice)
{
    this->firstSlice = firstSlice;
    this->lastSlice = lastSlice;
}

void SeedPropagater::propagate(std::vector<Seed> seeds)
{
    std::vector<Slice>::iterator it;
    for( it = firstSlice; it != lastSlice; it++ ) {
        Slice& slice = *it;
        cv::Mat& sliceImg = slice.getImg();

        for( Seed s : seeds ) {
            Seed newSeed(sliceImg, s.getId(), s.a, s.b);
            if( std::abs( newSeed.average - s.average ) < 2*s.relativeStdDev ) {
                slice.addSeed(newSeed);
            }
        }
    }
}
