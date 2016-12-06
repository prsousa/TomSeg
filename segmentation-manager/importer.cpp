#include "importer.h"

#include <fstream>

#include "json.hpp"

using json = nlohmann::json;

Importer::Importer()
{

}

Importer::Importer(SegmentationManager *segManager)
{
    this->segManager = segManager;
}

void Importer::importProject(std::string path)
{
    std::ifstream file;
    file.open( path, std::ios::binary);

    json proj;
    proj << file;

    if( proj.find("minimumFeatureSize") != proj.end() ) {
        segManager->setMinimumFeatureSize( proj["minimumFeatureSize"] );
    }

    if( proj.find("morphologicalSize") != proj.end() ) {
        segManager->setMorphologicalSize( proj["morphologicalSize"] );
    }

    if( proj.find("xLen") != proj.end() ) {
        segManager->setXLen( proj["xLen"] );
    }

    if( proj.find("yLen") != proj.end() ) {
        segManager->setYLen( proj["yLen"] );
    }

    if( proj.find("zLen") != proj.end() ) {
        segManager->setZLen( proj["zLen"] );
    }

    if( proj.find("useGPU") != proj.end() ) {
        segManager->setUseGPU( proj["useGPU"] );
    }

    if( proj.find("slices") != proj.end() ) {
        json slicesInfo = proj["slices"];

        for( json &sliceInfo : slicesInfo ) {
            if( sliceInfo.find("path") != sliceInfo.end() ) {
                std::string path = sliceInfo["path"];
                Slice slice(path);
                try {
                    Point a(sliceInfo["ROI"]["x"], sliceInfo["ROI"]["y"]);
                    slice.crop( a, sliceInfo["ROI"]["width"], sliceInfo["ROI"]["height"] );
                } catch(int e) {}

                if( sliceInfo.find("seeds") != sliceInfo.end() ) {
                    json seedsInfo = sliceInfo["seeds"];
                    for( json &seedInfo : seedsInfo ) {
                        try {
                            Point a(seedInfo["a"]["x"], seedInfo["a"]["y"]);
                            Point b(seedInfo["b"]["x"], seedInfo["b"]["y"]);
                            Seed s(slice.getImg(), seedInfo["id"], a, b);
                            slice.addSeed(s);
                        } catch(int e) {}
                    }
                }

                segManager->addSlice(slice);
            }
        }
    }

    file.close();
}
