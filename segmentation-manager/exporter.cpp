#include "exporter.h"

#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "json.hpp"

using json = nlohmann::json;

Exporter::Exporter()
{

}

Exporter::Exporter( SegmentationManager* segManager, size_t firstSlice, size_t lastSlice )
{
    this->segManager = segManager;

    std::vector<Slice>& slices = segManager->getSlices();
    this->startIndex = std::min( firstSlice - 1, slices.size() - 1 );
    this->endIndex = std::min( lastSlice, slices.size() );
}

void Exporter::exportResult( std::string path )
{
    std::vector<Slice>& slices = segManager->getSlices();

    size_t phaseNum = 0;
    for( size_t i = startIndex; i < endIndex; i++ ) {
        Slice& slice = slices[i];
        size_t slicePhaseNum = slice.getSeeds().size();
        if( phaseNum < slicePhaseNum ) {
            phaseNum = slicePhaseNum;
        }
    }

    std::ofstream *files = new std::ofstream[phaseNum];

    Slice& first = slices[startIndex];
    cv::Mat& firstImage = first.getImg();
    int size = endIndex - startIndex;
    int mode = 0;
    int nxstart, nystart, nzstart;
    nxstart = nystart = nzstart = 0;
    int mx = 1;
    int my = 1;
    int mz = 1;

    float xlen = segManager->getXLen();
    float ylen = segManager->getYLen();
    float zlen = segManager->getZLen();

    for( size_t i = 0; i < phaseNum; i++ ) {
        files[i].open( path + "/" + std::to_string(i) + ".mrc", std::ios::binary);

        files[i].write(reinterpret_cast<const char *>(&firstImage.cols), sizeof(int));
        files[i].write(reinterpret_cast<const char *>(&firstImage.rows), sizeof(int));
        files[i].write(reinterpret_cast<const char *>(&size), sizeof(int));
        files[i].write(reinterpret_cast<const char *>(&mode), sizeof(int));
        files[i].write(reinterpret_cast<const char *>(&nxstart), sizeof(int));
        files[i].write(reinterpret_cast<const char *>(&nystart), sizeof(int));
        files[i].write(reinterpret_cast<const char *>(&nzstart), sizeof(int));
        files[i].write(reinterpret_cast<const char *>(&mx), sizeof(int));
        files[i].write(reinterpret_cast<const char *>(&my), sizeof(int));
        files[i].write(reinterpret_cast<const char *>(&mz), sizeof(int));
        files[i].write(reinterpret_cast<const char *>(&xlen), sizeof(float));
        files[i].write(reinterpret_cast<const char *>(&ylen), sizeof(float));
        files[i].write(reinterpret_cast<const char *>(&zlen), sizeof(float));

        files[i].seekp (1024, std::ios::beg);
    }

    for( size_t i = startIndex; i < endIndex; i++ ) {
        Slice& slice = slices[i];
        const cv::Mat& segmentationResult = slice.getSegmentationResult();

        for( int y = 0; y < segmentationResult.rows; y++ ) {
            for( int x = 0; x < segmentationResult.cols; x++ ) {
                uchar label = segmentationResult.at<uchar>(y, x);

                for( size_t p = 0; p < phaseNum; p++ ) {
                    uchar val = (label == p) ? 1 : 0;
                    files[p].write(reinterpret_cast<const char *>(&val), sizeof(uchar));
                }
            }
        }
    }

    for( size_t i = 0; i < phaseNum; i++ ) {
        files[i].close();
    }

    delete[] files;
}

void Exporter::exportSlicesImages(std::string path, const std::string extension)
{
    std::vector<Slice>& slices = segManager->getSlices();

    char str[16];
    for( int i = startIndex; i < endIndex; i++ ) {
        Slice& slice = slices[i];

        const cv::Mat& image = slice.getImg();

        snprintf (str, 16, "%04d", i);

        cv::imwrite( path + "/" + str + extension, image );
    }
}

void Exporter::exportProject(std::string path)
{
    std::vector<Slice>& slices = segManager->getSlices();

    std::ofstream file;
    file.open( path, std::ios::binary );

    json proj;

    proj["minimumFeatureSize"] = segManager->getMinimumFeatureSize();
    proj["morphologicalSize"] = segManager->getMorphologicalSize();
    proj["xLen"] = segManager->getXLen();
    proj["yLen"] = segManager->getYLen();
    proj["zLen"] = segManager->getZLen();
    proj["useGPU"] = segManager->getUseGPU();

    json slicesInfo = json::array();

    for(Slice& slice : slices ) {
        json sliceInfo = json::object();
        sliceInfo["path"] = slice.getFilename();
        cv::Rect roiFromOriginal = slice.getRoiFromOriginal();
        sliceInfo["ROI"] = {
            { "x", roiFromOriginal.x },
            { "y", roiFromOriginal.y },
            { "width", roiFromOriginal.width },
            { "height", roiFromOriginal.height }
        };

        json seedsInfo = json::array();
        std::vector<Seed>& seeds = slice.getSeeds();
        for( Seed& seed : seeds ) {
            json seedInfo;
            seedInfo["id"] = seed.getId();
            seedInfo["a"]["x"] = seed.a.x;
            seedInfo["a"]["y"] = seed.a.y;
            seedInfo["b"]["x"] = seed.b.x;
            seedInfo["b"]["y"] = seed.b.y;

            seedsInfo.push_back(seedInfo);
        }

        sliceInfo["seeds"] = seedsInfo;

        slicesInfo.push_back( sliceInfo );
    }

    proj["slices"] = slicesInfo;


    file << proj.dump(2);

    file.close();
}
