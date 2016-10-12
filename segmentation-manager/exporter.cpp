#include "exporter.h"

#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

Exporter::Exporter()
{

}

Exporter::Exporter(std::vector<Slice>::iterator firstSlice, std::vector<Slice>::iterator lastSlice)
{
    this->firstSlice = firstSlice;
    this->lastSlice = lastSlice;
}

void Exporter::exportResult(std::string path, float xLen, float yLen, float zLen)
{
    size_t phaseNum = 0;
    size_t size = lastSlice - firstSlice;
    for( size_t i = 0; i < size; i++ ) {
        Slice& slice = *(firstSlice + i);
        size_t slicePhaseNum = slice.getSeeds().size();
        if( phaseNum < slicePhaseNum ) {
            phaseNum = slicePhaseNum;
        }
    }

    std::ofstream* files = new std::ofstream[phaseNum];

    Slice& first = *(firstSlice);
    cv::Mat& firstImage = first.getImg();
    int mode = 0;
    int nxstart, nystart, nzstart;
    nxstart = nystart = nzstart = 0;
    int mx = 1;
    int my = 1;
    int mz = 1;

    float xlen = xLen;
    float ylen = yLen;
    float zlen = zLen;

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

    for( size_t i = 0; i < size; i++ ) {
        Slice& slice = *(firstSlice + i);
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
}

void Exporter::exportSlicesImages(std::string path, const std::string extension)
{
    for( size_t i = 0; (i + this->firstSlice) != this->lastSlice; i++ ) {
        Slice& slice = *(i + this->firstSlice);

        const cv::Mat& image = slice.getImg();
        cv::imwrite( path + "/" + std::to_string(i) + extension, image );
    }
}
