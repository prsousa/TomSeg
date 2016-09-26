#ifndef CLIAPPLICATION_H
#define CLIAPPLICATION_H

#include "segmentation-manager/segmentationmanager.h"

class CliApplication
{
public:
    CliApplication(int argc, char* argv[]);
    int exec();

private:
    SegmentationManager segManager;

    void definePhasisSeeds(int sliceNumber);
    cv::Mat colorizeLabels(cv::Mat labels, std::vector<Seed> seeds);
};

#endif // CLIAPPLICATION_H
