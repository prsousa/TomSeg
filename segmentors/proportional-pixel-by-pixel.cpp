#include <iostream>

#include "proportional-pixel-by-pixel.h"
#include "../point.h"

using namespace std;

bool seedComparator (Seed i, Seed j) { return (i.average<j.average); }

ProportionalPixelByPixel::ProportionalPixelByPixel(cv::Mat img, std::vector<Seed> seeds) {
    this->img = img;
    this->seeds = seeds;

    std::sort (this->seeds.begin(), this->seeds.end(), seedComparator);
    this->intervals.push_back(0);

    for( int i = 0; i < this->seeds.size() - 1; i++ ) {
        int interval = this->seeds[i].stdDev * ((this->seeds[i+1].average - this->seeds[i].average) * 1.0f / (this->seeds[i].stdDev + this->seeds[i+1].stdDev) * 1.0f);
        cout << "SeedSorted #" << i << "\tμ: " << this->seeds[i].average << "\tσ: " << this->seeds[i].stdDev << "\t int: " << interval << endl;
        this->intervals.push_back( this->seeds[i].average + interval);
    }

    this->intervals.push_back(255);

    for(int interval : this->intervals) {
        cout << interval << endl;
    }
}

cv::Mat ProportionalPixelByPixel::Apply() {
    cv::Mat res(img.rows, img.cols, CV_8U);

    for(int i = 0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {

            int bluredIntensity = 0;
            {
                int blurSiz = 9 / 2;
                int n = 0;

                for(int b = max(i - blurSiz, 0); b <= min(i + blurSiz, img.rows); b++) {
                    for(int a = max(j - blurSiz, 0); a <= min(j + blurSiz, img.cols); a++) {
                        bluredIntensity += (int) img.at<uchar>(b, a);
                        n++;
                    }
                }

                bluredIntensity = bluredIntensity / n;
            }

            for(int k = 0; k < seeds.size(); k++) {
                if( bluredIntensity >= intervals[k] && bluredIntensity < intervals[k+1] ) {
                    res.at<uchar>(i, j) = seeds[k].id;
                    break;
                }
            }
        }
    }

    return res;
}
