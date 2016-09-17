#include "seed.h"

using namespace std;

void Seed::draw(cv::Mat img) {
    cv::Point a( this->a.x, this->a.y );
    cv::Point b( this->b.x, this->b.y );

    cv::rectangle( img, a, b, this->color, 3 );
}

std::ostream& operator<<(std::ostream& os, const Seed& s) {
    os << "Seed #" << s.id << "\t[(" << s.a.x << ", " << s.a.y << "), (" << s.b.x << ", " << s.b.y << ")]" << "\tμ: " << s.average << "\tσ: " << s.relativeStdDev;
    return os;
}

Seed* Seed::getSimilarSeed(vector<Seed>& seeds) {
    for( size_t i = 0; i < seeds.size(); i++ ) {
        Seed s = seeds[i];
        if( (this->average >= s.average - s.relativeStdDev) && (this->average <= s.average + s.relativeStdDev )) {
            // está dentro da média ± desvio padrão
            float relax = 0.05;
            if( (this->relativeStdDev >= s.relativeStdDev * (1-relax)) && (this->relativeStdDev <= s.relativeStdDev*(1+relax)) ) {
                return &seeds[i];
            }
        }
    }

    return NULL;
}

Seed Seed::getMoreSimilarSeedByAvg(std::vector<Seed>& seeds) {
    Seed res;
    int minorAvgDiff = INT_MAX;

    for( Seed s : seeds ) {
        int currentAvgDiff = std::abs( this->average - s.average );
        if( minorAvgDiff > currentAvgDiff ) {
            minorAvgDiff = currentAvgDiff;
            res = s;
        }
    }

    return res;
}

Seed Seed::getMoreSimilarSeedByStdDev(std::vector<Seed>&seeds) {
    Seed res;
    int minorStdDevDiff = INT_MAX;

    for( Seed s : seeds ) {
        int currentStdDevDiff = std::abs( this->relativeStdDev - s.relativeStdDev );
        if( minorStdDevDiff > currentStdDevDiff ) {
            minorStdDevDiff = currentStdDevDiff;
            res = s;
        }
    }

    return res;
}
