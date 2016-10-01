#ifndef DIFFERENTIATOR_H
#define DIFFERENTIATOR_H

#include <vector>

#include "../slice.h"

class Differentiator
{
public:
    Differentiator();
    Differentiator(std::vector<Slice>::iterator firstSlice, std::vector<Slice>::iterator lastSlice);
    void apply();

private:
    std::vector<Slice>::iterator firstSlice;
    std::vector<Slice>::iterator lastSlice;
};

#endif // DIFFERENTIATOR_H
