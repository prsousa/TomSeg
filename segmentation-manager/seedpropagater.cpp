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

        for( Seed& s : seeds ) {
            if( slice.containsSeedWithId(s.getId()) ) { continue; }

            Seed newSeed(sliceImg, s.getId(), s.a, s.b);
            if( std::abs( newSeed.average - s.average ) < 2*s.relativeStdDev ) {
                slice.addSeed(newSeed);
            } else {
                int seedHeight = s.b.y - s.a.y;
                int seedWidth = s.b.x - s.a.x;

                std::vector<Seed> possibleSeeds;
                for( int y = std::max(0, s.a.y - (int) (sliceImg.rows * 0.3)); ( y + seedHeight < sliceImg.rows ) && ( y < s.a.y + (sliceImg.rows * 0.3) ); y += seedHeight / 2 ) {
                    for( int x = std::max(0, s.a.x - (int) (sliceImg.cols * 0.3)); ( x + seedWidth < sliceImg.cols ) && ( x < s.a.x + (sliceImg.cols * 0.3) ); x += seedWidth / 2 ) {
                        Seed possibleSeed(sliceImg, s.getId(), Point(x, y), Point(x + seedWidth, y + seedHeight));
                        if( std::abs( possibleSeed.average - s.average ) < 2*s.relativeStdDev ) {
                            possibleSeeds.push_back( possibleSeed );
                        }
                    }
                }

                Seed* bestSeed = NULL;
                float bestDifference = 255;
                for( Seed& possibleSeed : possibleSeeds ) {
                    float currentDifference = std::abs( possibleSeed.average - s.average );
                    if( currentDifference < bestDifference ) {
                        bestDifference = currentDifference;
                        bestSeed = &possibleSeed;
                    }
                }

                if( bestSeed != NULL ) {
                    slice.addSeed( *bestSeed );
                } else {
                    // search with smaller seeds
                }

            }
        }
    }
}
