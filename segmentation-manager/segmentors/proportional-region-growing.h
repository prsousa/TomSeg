#ifndef PROPORTIONAL_REGION_GROWING_H
#define PROPORTIONAL_REGION_GROWING_H

#define USE_GPU_DEFAULT true

#include <unordered_map>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../slice.h"
#include "segmenter.h"


// GPU
void erodeAndDilate_GPU( unsigned char* labels, int size, int width, int height );

class ProportionalRegionGrowing : public Segmenter
{
private:
    cv::Mat img;
    std::vector<Seed> seeds;
    std::unordered_map< int, std::pair<int, int> > intervals;
    int minimumFeatureSize;
    int morphologicalSize;
    bool useGPU;

    void RegionGrowing( cv::Mat& res, Seed seed, bool (*pixelJudge)(int,void*), void* aditionalJudgeParams );
    void InitialConquer( cv::Mat& res );
    void AutomaticConquer( cv::Mat& res );
    void MorphologicalFiltering(cv::Mat& res);
    void FillTinyHoles(cv::Mat& res);

    bool FindNextSeed( Seed* res, cv::Mat labels, int minSize);
public:
    ProportionalRegionGrowing(Slice slice, int minimumFeatureSize = 15, int morphologicalSize = 15);
    ~ProportionalRegionGrowing();
    void setUseGPU(bool value);
    cv::Mat Apply();
};

#endif // PROPORTIONAL_REGION_GROWING_H
