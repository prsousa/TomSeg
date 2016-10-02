#ifndef ALIGNER_H
#define ALIGNER_H

#include <vector>

#include "../slice.h"

class Aligner
{
public:
    Aligner();
    Aligner(std::vector<Slice>::iterator firstSlice, std::vector<Slice>::iterator lastSlice);
    void apply(cv::Mat& masterImg, Point a, size_t width, size_t height);

private:
    std::vector<Slice>::iterator firstSlice;
    std::vector<Slice>::iterator lastSlice;
};

#endif // ALIGNER_H
