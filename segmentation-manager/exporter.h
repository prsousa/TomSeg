#ifndef EXPORTER_H
#define EXPORTER_H

#include <vector>
#include "slice.h"

class Exporter
{
public:
    Exporter();
    Exporter(std::vector<Slice>::iterator firstSlice, std::vector<Slice>::iterator lastSlice);
    void exportResult(std::string path);
    void exportSlicesImages(std::string path, const std::string extension = ".jpg");

private:
    std::vector<Slice>::iterator firstSlice;
    std::vector<Slice>::iterator lastSlice;
};

#endif // EXPORTER_H
