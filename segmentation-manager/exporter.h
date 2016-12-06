#ifndef EXPORTER_H
#define EXPORTER_H

#include <vector>
#include "segmentationmanager.h"

class Exporter
{
public:
    Exporter();
    Exporter(SegmentationManager* segManager, size_t firstSlice, size_t lastSlice);
    void exportResult(std::string path);
    void exportSlicesImages(std::string path, const std::string extension = ".jpg");
    void exportProject(std::string path);

private:
    SegmentationManager* segManager;
    size_t startIndex, endIndex;
};

#endif // EXPORTER_H
