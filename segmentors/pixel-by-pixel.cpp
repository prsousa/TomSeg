#include "pixel-by-pixel.h"

PixelByPixel::PixelByPixel(cv::Mat img, std::vector<Seed> seeds) {
    this->img = img;
    this->seeds = seeds;
}

cv::Mat PixelByPixel::Apply() {
    cv::Mat res(img.rows, img.cols, CV_8U);

    for(int i = 0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {

            uchar bestSeed_id = 0;
            uchar bestSeed_diff = 255;

            for(int k = 0; k < seeds.size(); k++) {
                uchar pixelIntensity = img.at<uchar>(i, j);
                uchar diff = abs(pixelIntensity - seeds[k].average);

                if( diff < bestSeed_diff ) {
                    bestSeed_diff = diff;
                    bestSeed_id = k;
                }
            }

            res.at<uchar>(i, j) = bestSeed_id;
        }
    }

    return res;
}
