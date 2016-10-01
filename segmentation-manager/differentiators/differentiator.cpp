#include "differentiator.h"

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

Differentiator::Differentiator()
{

}

Differentiator::Differentiator(std::vector<Slice>::iterator firstSlice, std::vector<Slice>::iterator lastSlice)
{
    this->firstSlice = firstSlice;
    this->lastSlice = lastSlice;
}

void displayImageApagar2(cv::string title, cv::Mat img, int x = 0, int y = 100) {
//    cv::imwrite("next_Seed.jpg", img);

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

void Differentiator::apply()
{
    std::vector<Slice>::iterator it;

    for( it = this->firstSlice + 1; it != this->lastSlice; it++ ) {
        Slice& slice0 = *(it - 1);
        Slice& slice1 = *it;

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
