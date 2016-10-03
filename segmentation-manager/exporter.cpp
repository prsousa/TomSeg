#include "exporter.h"

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

void Exporter::exportResult(std::string path)
{

}

void Exporter::exportSlicesImages(std::string path, const std::string extension)
{
    for( size_t i = 0; (i + this->firstSlice) != this->lastSlice; i++ ) {
        Slice& slice = *(i + this->firstSlice);

        const cv::Mat& image = slice.getImg();
        cv::imwrite( path + "/" + std::to_string(i) + extension, image );
    }
}
