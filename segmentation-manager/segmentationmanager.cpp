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

void displayImageApagar(string title, cv::Mat img, int x = 0, int y = 100);

cv::Mat translateImg(cv::Mat &img, int offsetx, int offsety){
    cv::Mat trans_mat = (cv::Mat_<double>(2,3) << 1, 0, offsetx, 0, 1, offsety);
    cv::warpAffine(img,img,trans_mat,img.size());
    return img;
}

void SegmentationManager::alignSlices(size_t masterSliceNumber, Point a, size_t width, size_t height )
{
    if( masterSliceNumber >= this->slices.size() ) return;

    Slice& masterSlice = this->slices[masterSliceNumber];
    cv::Mat& masterImg = masterSlice.getImg();
    cv::Rect roi(a.x, a.y, width, height);
    const cv::Mat templ = masterImg(roi);

    std::vector<Point> deltas(slices.size());
    int cutLeft = 0;
    int cutRight = 0;
    int cutUp = 0;
    int cutDown = 0;

    displayImageApagar("ROI", templ);
    cv::waitKey(0);

    for( size_t i = 0; i < slices.size(); i++) {
        if( i == masterSliceNumber ) continue;

        Slice& slice = this->slices[i];
        cv::Mat& sliceImg = slice.getImg();
        cv::Mat result;

        cv::matchTemplate( sliceImg, templ, result, cv::TM_CCOEFF_NORMED );
        cv::normalize( result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );

        double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
        cv::Point matchLoc;
        cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
        matchLoc = maxLoc;

        int deltaX = a.x - matchLoc.x;
        int deltaY = a.y - matchLoc.y;

        deltas[i] = Point(deltaX, deltaY);

        if( deltaX > 0 && deltaX > cutLeft ) {
            cutLeft = deltaX;
        }
        if( deltaX < 0 && abs(deltaX) > cutRight ) {
            cutRight = abs(deltaX);
        }
        if( deltaY > 0 && deltaY > cutUp ) {
            cutUp = deltaY;
        }
        if( deltaY < 0 && abs(deltaY) > cutDown ) {
            cutUp = abs(deltaY);
        }

        qDebug() << "x: " << deltas[i].x << "\ty: "<< deltas[i].y;
    }

    for( size_t i = 0; i < deltas.size(); i++ ) {
        Point delta = deltas[i];
        Slice& slice = this->slices[i];
        cv::Mat& sliceImg = slice.getImg();

        translateImg(sliceImg, delta.x, delta.y);
        cv::Rect cut(cutLeft, cutUp, sliceImg.cols - cutLeft - cutRight, sliceImg.rows - cutUp - cutDown);
        sliceImg = sliceImg(cut);
//        std::string name = "/Users/Paulo/Projetos/Tese/TomSeg/datasets/ROI/to_align/aligned/" + std::to_string(i) + ".jpg";
//        cv::imwrite(name, sliceImg);
    }

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

        Segmenter* segmenter = new ProportionalRegionGrowing(slice);
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
