#include "aligner.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QDebug>

Aligner::Aligner()
{

}

Aligner::Aligner(std::vector<Slice>::iterator firstSlice, std::vector<Slice>::iterator lastSlice)
{
    this->firstSlice = firstSlice;
    this->lastSlice = lastSlice;
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

    std::vector<Point> deltas(lastSlice - firstSlice);
    int cutLeft = 0;
    int cutRight = 0;
    int cutUp = 0;
    int cutDown = 0;

    std::vector<Slice>::iterator it;
    for( int i = 0; firstSlice + i != lastSlice; i++) {
        Slice& slice = *(i + firstSlice);
        cv::Mat& sliceImg = slice.getImg();
        cv::Mat matchResult;

        cv::matchTemplate( sliceImg, templ, matchResult, cv::TM_CCOEFF_NORMED );
        cv::normalize( matchResult, matchResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );

        double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
        cv::Point matchLoc;
        cv::minMaxLoc( matchResult, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
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
        Slice& slice = *(i + firstSlice);
        cv::Mat& sliceImg = slice.getImg();

        translateImg(sliceImg, delta.x, delta.y);
        cv::Rect cut(cutLeft, cutUp, sliceImg.cols - cutLeft - cutRight, sliceImg.rows - cutUp - cutDown);
        sliceImg = sliceImg(cut);
//        std::string name = "/Users/Paulo/Projetos/Tese/TomSeg/datasets/ROI/to_align/aligned/" + std::to_string(i) + ".jpg";
//        cv::imwrite(name, sliceImg);
    }

}
