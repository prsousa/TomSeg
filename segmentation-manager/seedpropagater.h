#ifndef SEEDPROPAGATER_H
#define SEEDPROPAGATER_H

#include <vector>
#include "slice.h"
#include "seed.h"

class SeedPropagater
{
public:
    SeedPropagater();
    SeedPropagater(std::vector<Slice>::iterator firstSlice, std::vector<Slice>::iterator lastSlice);
    void propagate(std::vector<Seed> seeds);

private:
    std::vector<Slice>::iterator firstSlice;
    std::vector<Slice>::iterator lastSlice;
};

#endif // SEEDPROPAGATER_H
