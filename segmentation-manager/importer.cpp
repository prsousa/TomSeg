#include "importer.h"

#include <fstream>

#include "json/json.h"

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
    file.open( path, std::ios::binary );

    Json::Value proj;
    file >> proj;

    if( proj["minimumFeatureSize"] != Json::nullValue ) {
        segManager->setMinimumFeatureSize( proj["minimumFeatureSize"].asInt() );
    }

    if( proj["morphologicalSize"] != Json::nullValue ) {
        segManager->setMorphologicalSize( proj["morphologicalSize"].asInt() );
    }

    if( proj["xLen"] != Json::nullValue ) {
        segManager->setXLen( proj["xLen"].asInt() );
    }

    if( proj["yLen"] != Json::nullValue) {
        segManager->setYLen( proj["yLen"].asInt() );
    }

    if( proj["zLen"] != Json::nullValue ) {
        segManager->setZLen( proj["zLen"].asInt() );
    }

    if( proj["useGPU"] != Json::nullValue ) {
        segManager->setUseGPU( proj["useGPU"].asBool() );
    }

    if( proj["slices"] != Json::nullValue ) {
        Json::Value slicesInfo = proj["slices"];

        for( Json::Value &sliceInfo : slicesInfo ) {
            if( sliceInfo["path"] != Json::nullValue ) {
                std::string path = sliceInfo["path"].asString();
                Slice slice(path);

                try {
                    Point a(sliceInfo["ROI"]["x"].asInt(), sliceInfo["ROI"]["y"].asInt());
                    slice.crop( a, sliceInfo["ROI"]["width"].asInt(), sliceInfo["ROI"]["height"].asInt() );
                } catch(int e) {}

                if( sliceInfo["seeds"] != Json::nullValue ) {
                    Json::Value seedsInfo = sliceInfo["seeds"];
                    for( Json::Value &seedInfo : seedsInfo ) {
                        try {
                            Point a(seedInfo["a"]["x"].asInt(), seedInfo["a"]["y"].asInt());
                            Point b(seedInfo["b"]["x"].asInt(), seedInfo["b"]["y"].asInt());
                            Seed s(slice.getImg(), seedInfo["id"].asInt(), a, b);
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
