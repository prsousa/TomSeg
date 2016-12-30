#include "aligner.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

Aligner::Aligner()
{

}

Aligner::Aligner(std::vector<Slice>::iterator firstSlice, std::vector<Slice>::iterator lastSlice, int maxDeltaX, int maxDeltaY)
{
    this->firstSlice = firstSlice;
    this->lastSlice = lastSlice;
    this->maxDeltaX = maxDeltaX;
    this->maxDeltaY = maxDeltaY;
}

void Aligner::apply()
{
    const int numSlices = lastSlice - firstSlice;
    std::vector<Point> deltas(numSlices);


    deltas[0] = Point(0, 0);
    #pragma omp parallel for shared(deltas) default(none)
    for( int i = 1; i < numSlices; i++ ) {
        Slice& prevSlice = *(i - 1 + firstSlice);
        Slice& slice = *(i + firstSlice);
        cv::Mat sliceImg = slice.getImg();

        this->maxDeltaX = 5;
        this->maxDeltaY = 10;

        int initROIX = this->maxDeltaX;
        int initROIY = this->maxDeltaY;
        int finalROIX = sliceImg.cols - 2*this->maxDeltaX;
        int finalROIY = sliceImg.rows - 2*this->maxDeltaY;

        cv::Rect searchAreaROI( initROIX, initROIY, finalROIX - initROIX, finalROIY - initROIY );
        cv::Mat templ = sliceImg( searchAreaROI );

        cv::Mat matchResult;
        cv::matchTemplate( prevSlice.getImg(), templ, matchResult, cv::TM_CCOEFF_NORMED );
        cv::normalize( matchResult, matchResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );

        double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
        cv::Point matchLoc;
        cv::minMaxLoc( matchResult, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
        matchLoc = maxLoc;

        deltas[i] = Point( matchLoc.x -  this->maxDeltaX, matchLoc.y - this->maxDeltaY );
    }

    for( int i = 1; i < numSlices; i++ ) {
        deltas[i].x += deltas[i-1].x;
        deltas[i].y += deltas[i-1].y;
    }

    applyDeltas( deltas );
}

void Aligner::apply(cv::Mat &masterImg, const Point a, const size_t width, const size_t height)
{
    cv::Rect roi(a.x, a.y, width, height);
    const cv::Mat templ = masterImg(roi);

    const int numSlices = lastSlice - firstSlice;

    std::vector<Point> deltas(numSlices);

    const int initROIX = std::max( 0, a.x - this->maxDeltaX );
    const int initROIY = std::max( 0, a.y - this->maxDeltaY );

    const int localTemplX = std::min( this->maxDeltaX, a.x );
    const int localTemplY = std::min( this->maxDeltaY, a.y );


    #pragma omp parallel for shared(deltas) default(none) // num_threads(8)
    for( int i = 0; i < numSlices; i++ ) {
        Slice& slice = *(i + firstSlice);
        cv::Mat sliceImg = slice.getImg();

        int finalROIX = std::min( sliceImg.cols, a.x + (int) width + this->maxDeltaX );
        int finalROIY = std::min( sliceImg.rows, a.y + (int) height + this->maxDeltaY );
        cv::Rect searchAreaROI( initROIX, initROIY, finalROIX - initROIX, finalROIY - initROIY );
        cv::Mat searchArea = sliceImg( searchAreaROI );

        cv::Mat matchResult;

        cv::matchTemplate( searchArea, templ, matchResult, cv::TM_CCOEFF_NORMED );
        cv::normalize( matchResult, matchResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );

        double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
        cv::Point matchLoc;
        cv::minMaxLoc( matchResult, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
        matchLoc = maxLoc;

        int deltaX = localTemplX - matchLoc.x;
        int deltaY = localTemplY - matchLoc.y;

        deltas[i] = Point(deltaX, deltaY);
    }

    applyDeltas( deltas );
}

void Aligner::applyDeltas(std::vector<Point> deltas)
{
    size_t numSlices = lastSlice - firstSlice;

    int maxRight = 0;
    int maxLeft = 0;
    int maxDown = 0;
    int maxUp = 0;

    for( size_t i = 0; i < numSlices; i++ ) {
        int deltaX = deltas[i].x;
        int deltaY = deltas[i].y;

        if( deltaX > 0 && deltaX > maxRight ) {
            maxRight = deltaX;
        }
        if( deltaX < 0 && std::abs(deltaX) > maxLeft ) {
            maxLeft = std::abs(deltaX);
        }
        if( deltaY > 0 && deltaY > maxDown ) {
            maxDown = deltaY;
        }
        if( deltaY < 0 && std::abs(deltaY) > maxUp ) {
            maxUp = std::abs(deltaY);
        }

        // std::cout << "x: " << deltaX << "\ty: "<< deltaY << std::endl;
    }

    #pragma omp parallel for // num_threads(8)
    for( size_t i = 0; i < numSlices; i++ ) {
        Point delta = deltas[i];
        Slice& slice = *(i + firstSlice);
        cv::Mat& sliceImg = slice.getImg();

        slice.crop( Point( maxRight - delta.x, maxDown - delta.y ), sliceImg.cols - maxLeft - maxRight, sliceImg.rows - maxUp - maxDown );
    }
}
