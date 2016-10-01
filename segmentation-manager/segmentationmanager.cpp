#include "segmentationmanager.h"

#include "segmentors/segmenter.h"
#include "segmentors/proportional-region-growing.h"
#include "differentiators/differentiator.h"

#include <QDebug>

using namespace std;

SegmentationManager::SegmentationManager()
{

}

void SegmentationManager::setSlices(vector<string> &filenames)
{
    slices.clear();

    vector<string>::iterator i;
    for( i = filenames.begin(); i != filenames.end(); i++ ) {
        string imagename = *i;
        slices.push_back( Slice(imagename) );
    }
}

void SegmentationManager::setSliceSeeds(size_t sliceNumber, const std::vector<Seed>& seeds)
{
    Slice& slice = this->slices[sliceNumber];
    slice.setSeeds( seeds );
}

std::vector<Slice> &SegmentationManager::getSlices()
{
    return this->slices;
}

Slice* SegmentationManager::getSlice(size_t sliceNumber)
{
    return &(this->slices[sliceNumber]);
}

void applyDifferences(vector<Slice>& slices)
{
    Differentiator dif(slices.begin(), slices.end());
    dif.apply();
}

cv::Mat SegmentationManager::apply(size_t sliceNumber)
{
    cv::Mat res;

    if( sliceNumber < this->slices.size() ) {
        Slice& slice = this->slices[sliceNumber];

        Segmenter* segmenter = new ProportionalRegionGrowing(slice.getImg(), slice.getSeeds(), slice.getMinimumFeatureSize());
        res = segmenter->Apply();

        slice.setSegmentationResult(res);

        delete segmenter;

        applyDifferences(this->slices);
    }

    return res;
}

bool SegmentationManager::isEmpty()
{
    return this->slices.empty();
}

size_t SegmentationManager::size()
{
    return slices.size();
}
