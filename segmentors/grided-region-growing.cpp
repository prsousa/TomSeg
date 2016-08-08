#include <iostream>

#include "grided-region-growing.h"
#include "../point.h"

using namespace std;

GridedRegionGrowing::GridedRegionGrowing(cv::Mat img, std::vector<Seed> seeds, int minimumPhaseSize) {
    this->img = img;
    this->seeds = seeds;

    this->minimumPhaseSize = minimumPhaseSize;
    this->cellSize = max(minimumPhaseSize / 2, 1);
    this->compressedImgMean = cv::Mat(img.rows / this->cellSize, img.cols / this->cellSize, CV_32F);
    this->compressedImgStdDev = cv::Mat(img.rows / this->cellSize, img.cols / this->cellSize, CV_32F);

//    cout << img.rows << " - " << img.cols << " - " << this->cellSize << " - " << this->compressedImgMean.rows << endl;

    for( int i = 0; i < img.rows; i += this->cellSize ) {
        for( int j = 0; j < img.cols; j += this->cellSize ) {
            Region r(img, Point(j, i), Point(min(j + this->cellSize, img.cols), min(i + this->cellSize, img.rows)));
            float avg = r.getAverageIntensity();
            this->compressedImgMean.at<float>(i / this->cellSize, j / this->cellSize) = avg;
            this->compressedImgStdDev.at<float>(i / this->cellSize, j / this->cellSize) = r.getStandardDeviation(avg);
        }
    }
}

cv::Mat GridedRegionGrowing::Apply() {
    cv::Mat res(img.rows, img.cols, CV_8U);
    res = cv::Scalar( EMPTY ); // start with no material

    for( int i = 0; i < compressedImgMean.rows; i++ ) {
        for( int j = 0; j < compressedImgMean.cols; j++ ) {

            float compressedMean = compressedImgMean.at<float>(i, j);

            for( int b = i * this->cellSize; b < min((i + 1) * this->cellSize, res.rows); b++ ) {
                for( int a = j * this->cellSize;  a < min((j + 1) * this->cellSize + 1, res.cols); a++) {
                    res.at<uchar>(b, a) = (uchar) compressedMean;
                }
            }
        }
    }

//    cout << compressedImgMean.cols << endl;
//    cout << img.cols << endl;
//    cout << res.cols << endl;

    cv::imshow("grid", res);
    cv::waitKey(0);

//    for(int k = 0; k < seeds.size(); k++) {
//        Seed seed = seeds[k];

//        vector<Point> queue;
//        cv::Mat visited(img.rows, img.cols, CV_8U);

//        for( int i = seed.a.y; i < seed.b.y; i++ ) {
//            queue.push_back(Point(seed.a.x - 1, i));
//            queue.push_back(Point(seed.b.x + 1, i));
//        }

//        for( int j = seed.a.x; j < seed.b.x; j++ ) {
//            queue.push_back(Point(j, seed.a.y - 1));
//            queue.push_back(Point(j, seed.b.y + 1));
//        }

//        while( !queue.empty() ) {
//            Point p = queue.back();
//            queue.pop_back();

//            if( p.y >= 0 && p.x >= 0 && p.y < img.rows && p.x < img.cols && !visited.at<uchar>(p.y, p.x) ) {
//                int bluredIntensity = 0;
//                {
//                    int blurSiz = 9 / 2;
//                    int n = 0;

//                    for(int b = max(p.y - blurSiz, 0); b <= min(p.y + blurSiz, img.rows); b++) {
//                        for(int a = max(p.x - blurSiz, 0); a <= min(p.x + blurSiz, img.cols); a++) {
//                            bluredIntensity += (int) img.at<uchar>(b, a);
//                            n++;
//                        }
//                    }

//                    bluredIntensity = bluredIntensity / n;
//                }


//                // also tried: use minium difference -> diff = min( "blured" value, original one)
//                // uchar diff = abs(bluredIntensity - seed.average);

//                if( bluredIntensity >= intervals[k] && bluredIntensity < intervals[k+1] ) {
//                    res.at<uchar>(p.y, p.x) = seed.id;

//                    queue.push_back(Point(p.x + 1, p.y));
//                    queue.push_back(Point(p.x - 1, p.y));
//                    queue.push_back(Point(p.x, p.y - 1));
//                    queue.push_back(Point(p.x, p.y + 1));

//                    // corners (8-way)
//                    queue.push_back(Point(p.x - 1, p.y - 1));
//                    queue.push_back(Point(p.x - 1, p.y + 1));
//                    queue.push_back(Point(p.x + 1, p.y - 1));
//                    queue.push_back(Point(p.x + 1, p.y + 1));
//                }

//                visited.at<uchar>(p.y, p.x) = 1;
//            }
//        }
//    }

    return res;
}
