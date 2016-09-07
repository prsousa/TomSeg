#include "seed.h"

using namespace std;

void Seed::draw(cv::Mat img) {
    cv::Point a( this->a.x, this->a.y );
    cv::Point b( this->b.x, this->b.y );

    cv::rectangle( img, a, b, this->color, 3 );
}

Seed* Seed::getSimmilarSeed(vector<Seed>& seeds) {
    for( Seed s : seeds ) {
        if( (this->average >= s.average - s.relativeStdDev) && (this->average <= s.average + s.relativeStdDev )) {
            // está dentro da média ± desvio padrão
            float relax = 0.05;
            if( (this->relativeStdDev >= s.relativeStdDev * (1-relax)) && (this->relativeStdDev <= s.relativeStdDev*(1+relax)) ) {
                return &s;
            }
        }
    }

    return NULL;
}
