#include <iostream>

#include "region-growing.h"
#include "../point.h"

using namespace std;

RegionGrowing::RegionGrowing(cv::Mat img, std::vector<Seed> seeds) {
    this->img = img;
    this->seeds = seeds;
}

cv::Mat RegionGrowing::Apply() {
    cv::Mat res(img.rows, img.cols, CV_8U);
    res = cv::Scalar( seeds.size() ); // start with no material

    for(int k = 0; k < seeds.size(); k++) {
        Seed seed = seeds[k];

        vector<Point> queue;
        cv::Mat visited(img.rows, img.cols, CV_8U);

        for( int i = seed.a.y; i < seed.b.y; i++ ) {
            queue.push_back(Point(seed.a.x - 1, i));
            queue.push_back(Point(seed.b.x + 1, i));
        }

        for( int j = seed.a.x; j < seed.b.x; j++ ) {
            queue.push_back(Point(j, seed.a.y - 1));
            queue.push_back(Point(j, seed.b.y + 1));
        }

        while( !queue.empty() ) {
            Point p = queue.back();
            queue.pop_back();

            if( p.y >= 0 && p.x >= 0 && p.y < img.rows && p.x < img.cols && !visited.at<uchar>(p.y, p.x) ) {
                int bluredIntensity = 0;
                int n = 0;
                {
                    int blurSiz = 9 / 2;

                    for(int b = max(p.y - blurSiz, 0); b <= min(p.y + blurSiz, img.rows); b++) {
                        for(int a = max(p.x - blurSiz, 0); a <= min(p.x + blurSiz, img.cols); a++) {
                            bluredIntensity += (int) img.at<uchar>(b, a);
                            n++;
                        }
                    }
                }

                bluredIntensity = bluredIntensity / n;

                // also tried: use minium difference -> diff = min( "blured" value, original one)
                uchar diff = abs(bluredIntensity - seed.average);

                if( diff <= 2 * seed.stdDev ) {
                    res.at<uchar>(p.y, p.x) = k;

                    queue.push_back(Point(p.x + 1, p.y));
                    queue.push_back(Point(p.x - 1, p.y));
                    queue.push_back(Point(p.x, p.y - 1));
                    queue.push_back(Point(p.x, p.y + 1));

                    // corners (8-way)
                    queue.push_back(Point(p.x - 1, p.y - 1));
                    queue.push_back(Point(p.x - 1, p.y + 1));
                    queue.push_back(Point(p.x + 1, p.y - 1));
                    queue.push_back(Point(p.x + 1, p.y + 1));
                }

                visited.at<uchar>(p.y, p.x) = 1;
            }
        }
    }

    return res;
}