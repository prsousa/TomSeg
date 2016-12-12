#include "cliapplication.h"
#include <iostream>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <boost/program_options.hpp>
#include <vector>

namespace po = boost::program_options;

void onSliceChange( int a, void* data ) {
    SegmentationManager* segManager = (SegmentationManager*) data;
    int numSlices = segManager->size();

    if( a >= numSlices ) { return; }

    Slice* slice = segManager->getSlice(a);
    cv::Mat& image = slice->getImg();

    cv::Mat imgResized;
    if( image.rows > 1000 ) {
        int newHight = 600;
        int newWidth = image.cols * newHight / image.rows;
        cv::resize(image, imgResized, cv::Size(newWidth, newHight));
    } else {
        imgResized = image;
    }

    cv::imshow( "SegImage", imgResized );
}

class SliceDisplayer
{
    public:
    SliceDisplayer(SegmentationManager* segManager) {
        this->segManager = segManager;
        current = 0;
    }

    void display() {
        size_t numSlices = segManager->size();

        cv::namedWindow( "SegImage", cv::WINDOW_AUTOSIZE );// Create a window for display.

        char trackbarName[50];
        sprintf( trackbarName, "Slice (%zu): ", numSlices );

        cv::createTrackbar(trackbarName, "SegImage", &current, numSlices - 1, onSliceChange, segManager);

        onSliceChange(0, segManager);
        cv::waitKey(0);
    }

private:
    int current;
    SegmentationManager* segManager;
};


CliApplication::CliApplication( int argc, char *argv[] )
{
    this->argc = argc;
    this->argv = argv;
}

cv::Mat CliApplication::colorizeLabels(cv::Mat labels)
{
    cv::Mat res(labels.rows, labels.cols, CV_8UC3);
    res = cv::Scalar( 0 );

    std::vector<cv::Vec3b> colorTable( 256 ); // could be static
    uchar color[3];

    for(int i = 0; i < 256; i++) {
        Seed::getColor(i, color);
        colorTable[i][0] = color[0];
        colorTable[i][1] = color[1];
        colorTable[i][2] = color[2];
    }

    for(int i = 0; i < labels.rows; i++) {
        for(int j = 0; j < labels.cols; j++) {
            int label = labels.at<uchar>(i, j);
            if(label != EMPTY) {
                res.at<cv::Vec3b>(i, j) = colorTable[label];
            }
        }
    }

    return res;
}

int CliApplication::exec()
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("images,i", po::value< std::vector<std::string> >(), "images to import")
        ("project,p", po::value< std::string >(), "project file")
        ("segment,s", "segments the volume")
        ("display,d", "displays the result in a basic GUI")
        ("output,o", po::value< std::string >(), "path to resulting *.mrc file")
        ("export,e", po::value< std::string >(), "folder path to exporting image slices")
    ;

    po::positional_options_description p;
    p.add("images", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(this->argc, this->argv).
              options(desc).positional(p).run(), vm);
    po::notify(vm);

    if ( vm.count("help") ) {
        std::cout << "Usage: TomSeg [options]\n";
        std::cout << desc;
        return EXIT_SUCCESS;
    }

    if ( vm.count("images") && !vm.count("project") ) {
        segManager = SegmentationManager();
        std::vector<std::string> filenames =  vm["images"].as< std::vector<std::string> >();
        segManager.setSlices( filenames );
    }

    if( vm.count("project") ) {
        segManager = SegmentationManager( vm["project"].as< std::string >() );
    }


    if( segManager.isEmpty() ) {
        return -1;
    }

    if( vm.count("segment") ) {
        segManager.segment();
    }

    if( vm.count("output") ) {
        segManager.exportResult( vm["output"].as< std::string >() );
    }

    if( vm.count("export") ) {
        segManager.exportSlicesImages( vm["export"].as< std::string >() );
    }

    if( vm.count("display") ) {
        SliceDisplayer displayer(&segManager);
        displayer.display();
    }

    return EXIT_SUCCESS;
}
