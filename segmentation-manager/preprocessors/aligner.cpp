#include "aligner.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QDebug>

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
    int numSlices = lastSlice - firstSlice;
    std::vector<Point> deltas(numSlices);


    deltas[0] = Point(0, 0);

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

cv::Mat translateImg(cv::Mat &img, int offsetx, int offsety){
    cv::Mat trans_mat = (cv::Mat_<double>(2,3) << 1, 0, offsetx, 0, 1, offsety);
    cv::warpAffine(img,img,trans_mat,img.size());
    return img;
}

void Aligner::apply(cv::Mat &masterImg, Point a, size_t width, size_t height)
{
    cv::Rect roi(a.x, a.y, width, height);
    const cv::Mat templ = masterImg(roi);

    int numSlices = lastSlice - firstSlice;

    std::vector<Point> deltas(numSlices);

    std::vector<Slice>::iterator it;
    #pragma omp parallel for shared(templ) // num_threads(8)
    for( int i = 0; i < numSlices; i++ ) {
        Slice& slice = *(i + firstSlice);
        cv::Mat sliceImg = slice.getImg();

        int initROIX = std::max( 0, a.x - this->maxDeltaX );
        int initROIY = std::max( 0, a.y - this->maxDeltaY );
        int finalROIX = std::min( sliceImg.cols, a.x + (int) width + this->maxDeltaX );
        int finalROIY = std::min( sliceImg.rows, a.y + (int) height + this->maxDeltaY );

        int localTemplX = std::min( this->maxDeltaX, a.x );
        int localTemplY = std::min( this->maxDeltaY, a.y );

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
    int numSlices = lastSlice - firstSlice;

    int cutLeft = 0;
    int cutRight = 0;
    int cutUp = 0;
    int cutDown = 0;

    for( int i = 0; i < numSlices; i++ ) {
        int deltaX = deltas[i].x;
        int deltaY = deltas[i].y;

        if( deltaX > 0 && deltaX > cutLeft ) {
            cutLeft = deltaX;
        }
        if( deltaX < 0 && std::abs(deltaX) > cutRight ) {
            cutRight = std::abs(deltaX);
        }
        if( deltaY > 0 && deltaY > cutUp ) {
            cutUp = deltaY;
        }
        if( deltaY < 0 && std::abs(deltaY) > cutDown ) {
            cutDown = std::abs(deltaY);
        }

        qDebug() << "x: " <<deltaX << "\ty: "<< deltaY;
    }

    #pragma omp parallel for // num_threads(8)
    for( size_t i = 0; i < numSlices; i++ ) {
        Point delta = deltas[i];
        Slice& slice = *(i + firstSlice);
        cv::Mat& sliceImg = slice.getImg();
    //        std::string name = "/Users/Paulo/Projetos/Tese/TomSeg/datasets/ROI/to_align/original/" + std::to_string(i) + ".jpg";
    //        cv::imwrite(name, sliceImg);


        translateImg(sliceImg, delta.x, delta.y);
        cv::Rect cut(cutLeft, cutUp, sliceImg.cols - cutLeft - cutRight, sliceImg.rows - cutUp - cutDown);
        sliceImg = sliceImg(cut);
    //        name = "/Users/Paulo/Projetos/Tese/TomSeg/datasets/ROI/to_align/aligned/" + std::to_string(i) + ".jpg";
    //        cv::imwrite(name, sliceImg);
    }
}
