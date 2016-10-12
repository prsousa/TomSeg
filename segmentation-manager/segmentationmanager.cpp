#include "segmentationmanager.h"

#include "segmentors/segmenter.h"
#include "segmentors/proportional-region-growing.h"
#include "differentiators/differentiator.h"
#include "preprocessors/aligner.h"
#include "exporter.h"
#include "seedpropagater.h"

#include <QDebug>

using namespace std;

SegmentationManager::SegmentationManager()
{
    xLen = yLen = zLen = 1.f;
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

void SegmentationManager::alignSlices(size_t masterSliceNumber, Point a, size_t width, size_t height, int maxDeltaX, int maxDeltaY )
{
    if( masterSliceNumber < this->slices.size() ) {
        Slice& masterSlice = this->slices[masterSliceNumber];

        Aligner aligner(this->slices.begin(), this->slices.end(), maxDeltaX, maxDeltaY);
        aligner.apply( masterSlice.getImg(), a, width, height );
    }
}

void SegmentationManager::cropSlices(size_t firstSlice, size_t lastSlice, Point a, size_t width, size_t height)
{
    firstSlice--;
    lastSlice--;

    vector<Slice>::iterator cutStrt = slices.begin();
    vector<Slice>::iterator cutStop = cutStrt + firstSlice;

    if( firstSlice > 0 ) {
        slices.erase(slices.begin(), slices.begin() + firstSlice);
    }

    cutStrt = slices.begin() + (lastSlice - firstSlice + 1);
    cutStop = slices.end();

    if( cutStrt < cutStop ) {
        slices.erase(cutStrt, cutStop);
    }

    for( Slice& slice : slices ) {
        slice.crop(a, width, height);
    }
}

void SegmentationManager::resetResults()
{
    for( Slice& slice : slices ) {
        slice.resetSegmentationResult();
    }
}

void SegmentationManager::exportResult(string path)
{
    this->exportResult(path, 1, this->size());
}

void SegmentationManager::exportResult(string path, size_t firstSlice, size_t lastSlice)
{
    if (lastSlice < firstSlice ) return;

    size_t startOffset = std::min( firstSlice - 1,  this->slices.size() - 1 );
    size_t endOffset = std::min( lastSlice,  this->slices.size() );

    Exporter exporter( this->slices.begin() + startOffset, this->slices.begin() + endOffset );
    exporter.exportResult(path, this->xLen, this->yLen, this->zLen);
}

void SegmentationManager::exportSlicesImages(string path)
{
    this->exportSlicesImages(path, 1, this->size());
}

void SegmentationManager::exportSlicesImages(string path, size_t firstSlice, size_t lastSlice)
{
    if (lastSlice < firstSlice ) return;

    size_t startOffset = std::min( firstSlice - 1,  this->slices.size() - 1 );
    size_t endOffset = std::min( lastSlice,  this->slices.size() );

    Exporter exporter( this->slices.begin() + startOffset, this->slices.begin() + endOffset );
    exporter.exportSlicesImages(path);
}

std::vector<Slice> &SegmentationManager::getSlices()
{
    return this->slices;
}

Slice* SegmentationManager::getSlice(size_t sliceNumber)
{
    return &(this->slices[sliceNumber]);
}

void SegmentationManager::propagateSeeds(size_t sliceNumber)
{
    if( sliceNumber < this->slices.size() ) {
        Slice& masterSlice = this->slices[sliceNumber];
        if( masterSlice.seedsNumber() >= 2 ) {
            SeedPropagater seedPropagater( this->slices.begin() + sliceNumber + 1, this->slices.end() );
            seedPropagater.propagate( masterSlice.getSeeds() );
        }
    }
}

void applyDifferences(std::vector<Slice>::iterator firstSlice, std::vector<Slice>::iterator lastSlice)
{
    Differentiator dif(firstSlice, lastSlice);
    dif.apply();
}

cv::Mat SegmentationManager::segment()
{
    cv::Mat res;
    std::vector<int> slicesWithSeedsIndex;
    {
        int i = 0;
        for( Slice& slice : this->slices ) {
            if( slice.seedsNumber() >= 2 ) {
                slicesWithSeedsIndex.push_back(i);
            }
            i++;
        }
    }

    slicesWithSeedsIndex.push_back( this->slices.size() ); // "hack" to make intervals work

    for( int i = 0; i < slicesWithSeedsIndex.size() - 1; i++ ) {
        int firstSliceIndex = slicesWithSeedsIndex[i];
        int lastSliceIndex = slicesWithSeedsIndex[i + 1];

        Slice& slice = this->slices[ firstSliceIndex ];
        ProportionalRegionGrowing segmenter(slice);
        res = segmenter.Apply();
        slice.setSegmentationResult(res);

        applyDifferences(this->slices.begin() + firstSliceIndex, this->slices.begin() + lastSliceIndex);

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

void SegmentationManager::setXLen(float value)
{
    xLen = value;
}

void SegmentationManager::setYLen(float value)
{
    yLen = value;
}

void SegmentationManager::setZLen(float value)
{
    zLen = value;
}
