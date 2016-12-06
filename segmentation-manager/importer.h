#ifndef IMPORTER_H
#define IMPORTER_H

#include "segmentationmanager.h"

class Importer
{
public:
    Importer();
    Importer(SegmentationManager* segManager);

    void importProject(std::string path);

private:
    SegmentationManager* segManager;
};

#endif // IMPORTER_H
