#include "segmentationmanager.h"

#include "segmentors/segmenter.h"
#include "segmentors/proportional-region-growing.h"

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

Slice* SegmentationManager::getSlice(size_t sliceNumber)
{
    return &(this->slices[sliceNumber]);
}

int* SegmentationManager::apply(size_t sliceNumber)
{
    if( sliceNumber < 0 || sliceNumber >= this->slices.size() ) {
        return NULL;
    }

    Slice& slice = this->slices[sliceNumber];

    Segmenter* segmenter = new ProportionalRegionGrowing(slice.getImg(), slice.getSeeds(), slice.getMinimumFeatureSize());
    cv::Mat labels = segmenter->Apply();
    slice.setSegmentationResult( labels );
    delete segmenter;

    int* res = new int[labels.rows*labels.cols];

    // qDebug() << "cv::Mat " << labels.rows << "rows; " << labels.cols << "cols;";

    for( int y = 0; y < labels.rows; y++) {
        for( int x = 0; x < labels.cols; x++) {
            res[ y*labels.cols + x] = labels.at<uchar>(y, x);
        }
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
