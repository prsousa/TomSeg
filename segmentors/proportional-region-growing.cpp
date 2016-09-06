#include <iostream>

#include "proportional-region-growing.h"
#include "../point.h"

using namespace std;

bool seedComparator (Seed i, Seed j) { return (i.average<j.average); }

ProportionalRegionGrowing::ProportionalRegionGrowing(cv::Mat img, std::vector<Seed> seeds) {
    this->img = img;
    this->seeds = seeds;

    std::sort (this->seeds.begin(), this->seeds.end(), seedComparator);
    this->intervals.push_back(0);

    for( int i = 0; i < this->seeds.size() - 1; i++ ) {
        int interval = this->seeds[i].relativeStdDev * ((this->seeds[i+1].average - this->seeds[i].average) * 1.0f / (this->seeds[i].relativeStdDev + this->seeds[i+1].relativeStdDev) * 1.0f);
//        cout << "SeedSorted #" << i << "\tμ: " << this->seeds[i].average << "\tσ: " << this->seeds[i].stdDev << "\t int: " << interval << endl;
        this->intervals.push_back( this->seeds[i].average + interval);
    }

    this->intervals.push_back(255);

    for(int interval : this->intervals) {
        cout << interval << endl;
    }
}

cv::Mat Erode(cv::Mat masks, int size) {
    cv::Mat res;
    masks.copyTo(res);

    size = size / 2;

    for( int i = 0; i < masks.rows; i++ ) {
        for( int j = 0; j < masks.cols; j++ ) {

            bool fits = true;

            for( int y = max(0, i - size); fits && y < min(masks.rows, i + size); y++ ) {
                for( int x = max(0, j - size); fits && x < min(masks.cols, j + size); x++ ) {
                    fits = masks.at<uchar>(y, x) == masks.at<uchar>(i, j);
                }
            }

            if( !fits ) {
                res.at<uchar>(i, j) = EMPTY;
            }

        }
    }

    return res;
}

cv::Mat Dilate(cv::Mat masks, int size) {
    cv::Mat res;
    masks.copyTo(res);

    size = size / 2;

    for( int i = 0; i < masks.rows; i++ ) {
        for( int j = 0; j < masks.cols; j++ ) {

            uchar color = EMPTY;

            for( int y = max(0, i - size); y < min(masks.rows, i + size); y++ ) {
                for( int x = max(0, j - size); x < min(masks.cols, j + size); x++ ) {
                    color = min(color, masks.at<uchar>(y, x));
                }
            }

            res.at<uchar>(i, j) = color;

        }
    }

    return res;
}

Seed ProportionalRegionGrowing::FindNextSeed( cv::Mat labels, int minSize ) {

    for(int y = 0; y < img.rows - minSize; y++) {
        for(int x = 0; x < img.cols - minSize; x++) {
            Point a(x, y);
            Point b(x + minSize, y + minSize);
            Region labelRegion(labels, a, b);

            if( labelRegion.isColor<uchar>(EMPTY) ) {
                Region imgRegion(this->img, a, b);
                std::cout << "centerOfMass" << std::endl;
                if( imgRegion.centerOfMassIsMiddle() ) {
                    return Seed(this->img, a, b);
                }
            }
        }
    }


    return Seed(this->img, Point(0, 0), Point(10, 10));
}

void displayImageApagar(string title, cv::Mat img, int x = 0, int y = 100) {
    if( img.rows > 1000 ) {
        int newHigh = 600;
        int newWidth = img.cols * newHigh / img.rows;
        cv::resize(img, img, cv::Size(newWidth, newHigh));
    }

    cv::namedWindow(title, cv::WINDOW_AUTOSIZE);
    cv::moveWindow(title, x, y);
    cv::imshow(title, img);
}

// TODO: this solution to improve the modularity may not be the best
// instead of receiving the aditionalJudgeParams arg, it might be better
// to create an object ProportinalJudge that encapsulates it
void ProportionalRegionGrowing::RegionGrowing( cv::Mat& res, Seed seed, bool (*pixelJudge)(int,void*), void* aditionalJudgeParams ) {

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
            {
                int blurSiz = 9 / 2;
                int n = 0;

                for(int b = max(p.y - blurSiz, 0); b <= min(p.y + blurSiz, img.rows); b++) {
                    for(int a = max(p.x - blurSiz, 0); a <= min(p.x + blurSiz, img.cols); a++) {
                        bluredIntensity += (int) img.at<uchar>(b, a);
                        n++;
                    }
                }

                bluredIntensity = bluredIntensity / n;
            }

            if( (*pixelJudge)(bluredIntensity, aditionalJudgeParams) ) {
                res.at<uchar>(p.y, p.x) = seed.id;

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

bool proportionalJudge(int intensity, void* aditionalJudgeParams) {
    int* judgeParams = (int*) aditionalJudgeParams;
    return ( intensity >= judgeParams[0] && intensity < judgeParams[1]);
}

cv::Mat ProportionalRegionGrowing::Apply() {
    cv::Mat res(img.rows, img.cols, CV_8U);
    res = cv::Scalar( EMPTY ); // start with no material

    int proportionalSeedIntervals[2];
    for(int k = 0; k < seeds.size(); k++) {
        Seed seed = seeds[k];

        proportionalSeedIntervals[0] = intervals[k];
        proportionalSeedIntervals[1] = intervals[k+1];

        this->RegionGrowing( res, seed, proportionalJudge, proportionalSeedIntervals );
    }

    int morphSize = 15;
    res = Erode(res, morphSize);
    res = Dilate(res, morphSize);

    Seed nextSeed = this->FindNextSeed( res, 35 );

    cv::Mat imgWithNewSeed;
    cv::cvtColor(this->img, imgWithNewSeed, cv::COLOR_GRAY2BGR);
    nextSeed.draw(imgWithNewSeed);

    displayImageApagar("New Seed", imgWithNewSeed);
    cv::waitKey();

    return res;
}
