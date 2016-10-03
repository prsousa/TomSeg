#ifndef ALIGNER_H
#define ALIGNER_H

#include <vector>

#include "../slice.h"

class Aligner
{
public:
    Aligner();
    Aligner(std::vector<Slice>::iterator firstSlice, std::vector<Slice>::iterator lastSlice, int maxDeltaX = INT_MAX, int maxDeltaY = INT_MAX);
    void apply(cv::Mat& masterImg, Point a, size_t width, size_t height);

private:
    std::vector<Slice>::iterator firstSlice;
    std::vector<Slice>::iterator lastSlice;
    int maxDeltaX, maxDeltaY;
};

#endif // ALIGNER_H
