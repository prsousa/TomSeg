#ifndef PROPORTIONAL_REGION_GROWING_H
#define PROPORTIONAL_REGION_GROWING_H

#include <unordered_map>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../seed.h"
#include "segmenter.h"

class ProportionalRegionGrowing : public Segmenter
{
private:
    cv::Mat img;
    std::vector<Seed> seeds;
    std::unordered_map< int, std::pair<int, int> > intervals;

    void RegionGrowing( cv::Mat& res, Seed seed, bool (*pixelJudge)(int,void*), void* aditionalJudgeParams );
    void InitialConquer( cv::Mat& res );
    void AutomaticConquer( cv::Mat& res );
    void MorphologicalFiltering(cv::Mat& res, int morphSize);
    bool FindNextSeed( Seed* res, cv::Mat labels, int minSize);
public:
    ProportionalRegionGrowing(cv::Mat img, std::vector<Seed> seeds);
    cv::Mat Apply();
};

#endif // PROPORTIONAL_REGION_GROWING_H
