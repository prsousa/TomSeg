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

std::vector<Slice> &SegmentationManager::getSlices()
{
    return this->slices;
}

Slice* SegmentationManager::getSlice(size_t sliceNumber)
{
    return &(this->slices[sliceNumber]);
}


void displayImageApagar2(string title, cv::Mat img, int x = 0, int y = 100) {
    cv::imwrite("next_Seed.jpg", img);

    if( img.rows > 1000 ) {
        int newHigh = 600;
        int newWidth = img.cols * newHigh / img.rows;
        cv::resize(img, img, cv::Size(newWidth, newHigh));
    }

    cv::namedWindow(title, cv::WINDOW_AUTOSIZE);
    cv::moveWindow(title, x, y);
    cv::imshow(title, img);

    cv::waitKey();
}

void applyDifferences(vector<Slice>& slices)
{

    for( int i = 1; i < slices.size(); i++ ) {
        Slice& slice0 = slices[i-1];
        Slice& slice1 = slices[i];

        cv::Mat img0 = slice0.getImg();
        cv::Mat img1 = slice1.getImg();

        // displayImageApagar2("Slice 0", slice0 );
        // displayImageApagar2("Slice 1", slice1 );
        cv::Mat diff = abs( img0 - img1 );
        // displayImageApagar2("Diff", diff);

        cv::threshold( diff, diff, 30, 255, 0 );

        // displayImageApagar2("Diff", diff);

        int morph_size = 3;
        cv::Mat element = cv::getStructuringElement( cv::MORPH_RECT, cv::Size( 2*morph_size + 1, 2*morph_size+1 ), cv::Point( morph_size, morph_size ) );
        cv::morphologyEx( diff, diff, cv::MORPH_OPEN, element, cv::Point(-1,-1), 1 );
        // displayImageApagar2("Diff", diff);

        cv::Mat res1 = slice0.getSegmentationResult().clone();
        for( int y = 0; y < res1.rows; y++ ) {
            for( int x = 0; x < res1.cols; x++ ) {
                if( diff.at<uchar>(y, x) == 255 ) {
                    res1.at<uchar>(y, x) = 0;
                }
            }
        }

        slice1.setSegmentationResult( res1 );
    }
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
