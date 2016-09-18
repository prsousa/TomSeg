#ifndef REGION_H
#define REGION_H

#include <opencv2/core/core.hpp>

#include "point.h"

class Region
{
public:
    Point a, b;
    cv::Mat* img;

    Region() {}

    Region(cv::Mat& img, Point a, Point b) {
        assert(a.x >= 0 && a.x <= img.cols && a.y >= 0 && a.y <= img.rows);
        assert(b.x >= 0 && b.x <= img.cols && b.y >= 0 && b.y <= img.rows);
        assert(a.x < b.x && a.y < b.y);

//        std::cout << a.x << " " << a.y << std::endl;

        this->img = &img;
        this->a = a;
        this->b = b;
    }

    inline int getNumberPixeis() {
        return ( (b.x - a.x) * (b.y - a.y) );
    }

    float getAverageIntensity() {
        int accum = 0;

        for(int y = a.y; y < b.y; y++) {
            for(int x = a.x; x < b.x; x++) {
                accum += img->at<uchar>(y, x);
            }
        }

        return accum / this->getNumberPixeis();
    }

    float getStandardDeviation(float average) {
        int accum = 0;

        for(int y = a.y; y < b.y; y++) {
            for(int x = a.x; x < b.x; x++) {
                accum += pow(img->at<uchar>(y, x) - average, 2.0f);
            }
        }

        return sqrt( accum / this->getNumberPixeis() );
    }

    template<class T>
    bool isColor( T color ) {
        bool res = true;
        for(int y = this->a.y; res && y < this->b.y; y++) {
            for(int x = this->a.x; res && x < this->b.x; x++) {
                res = ( this->img->at<T>(y, x) == color );
            }
        }

        return res;
    }

    Point centerOfMass() {
        int sumProductX = 0;
        int sumProductY = 0;
        int sum = 0;

        for(int y = this->a.y; y < this->b.y; y++) {
            for(int x = this->a.x; x < this->b.x; x++) {
                int p = this->img->at<uchar>(y, x);
                sumProductX += x * p;
                sumProductY += y * p;
                sum += p;
            }
        }

        if( !sum ) return Point( 0, 0);
        return Point(sumProductX / sum, sumProductY / sum);
    }

    bool centerOfMassIsMiddle(int marginOfError = 5) {
        assert( marginOfError >= 0 && marginOfError <= 100);
        Point centerOfMass = this->centerOfMass();
        int midX = (this->a.x + this->b.x) / 2;
        int midY = (this->a.y + this->b.y) / 2;

        int errorX = (marginOfError / 100.f) * (this->b.x - this->a.x);
        int errorY = (marginOfError / 100.f) * (this->b.y - this->a.y);

        return centerOfMass.x >= (midX - errorX) && centerOfMass.x <= (midX + errorX)
                && centerOfMass. y >= (midY - errorY) && centerOfMass.y <= (midY + errorY);
    }
};

#endif // REGION_H
