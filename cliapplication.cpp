#include "cliapplication.h"
#include <iostream>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <boost/program_options.hpp>
#include <vector>

namespace po = boost::program_options;

void onSliceChange( int a, void* data );
static int windowId = 0;

class SliceDisplayer
{
    public:
    SliceDisplayer(SegmentationManager* segManager) : SliceDisplayer( segManager, "SegResult") {
        this->windowName += "_" + std::to_string(windowId++);
    }

    SliceDisplayer(SegmentationManager* segManager, std::string windowName ) {
        this->segManager = segManager;
        this->windowName = windowName;
        this->current = 0;
        this->labelsAlpha = 70;
    }

    void display() {
        size_t numSlices = segManager->size();
        cv::namedWindow( windowName, cv::WINDOW_AUTOSIZE );// Create a window for display.

        std::string sliceTrackbarName = "Slice (" + std::to_string(numSlices) + ")";

        cv::createTrackbar(sliceTrackbarName, windowName, &current, numSlices - 1, onSliceChange, this);
        cv::createTrackbar("Alpha", windowName, &this->labelsAlpha, 100, onSliceChange, this);

        onSliceChange(0, this);
        cv::waitKey(0);
    }

    SegmentationManager* getSegmentationManager() const {
        return this->segManager;
    }

    std::string getWindowName() const {
        return this->windowName;
    }

    int getLabelsAlpha() const {
        return labelsAlpha;
    }

    int getCurrent() const {
        return current;
    }

private:
    int current;
    std::string windowName;
    int labelsAlpha;
    SegmentationManager* segManager;
};

cv::Mat colorizeLabels(cv::Mat& labels)
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

void onSliceChange( int a, void* data ) {
    SliceDisplayer* sliceDisplayer = (SliceDisplayer*) data;
    SegmentationManager* segManager = sliceDisplayer->getSegmentationManager();

    Slice* slice = segManager->getSlice( sliceDisplayer->getCurrent() );
    cv::Mat& image = slice->getImg();
    cv::Mat& segResult = slice->getSegmentationResult();
    cv::Mat labels = colorizeLabels( segResult );

    cv::Mat imgResized, labelsResized;
    if( image.rows > 1000 ) {
        int newHight = 600;
        int newWidth = image.cols * newHight / image.rows;
        cv::resize(image, imgResized, cv::Size(newWidth, newHight));
        if( !labels.empty() ) cv::resize(labels, labelsResized, cv::Size(newWidth, newHight));
    } else {
        imgResized = image;
        labelsResized = labels;
    }

    cv::Mat imgShow;
    if( !labelsResized.empty() ) {
        cv::Mat imgCombined;
        float alpha = 1.0 - ( sliceDisplayer->getLabelsAlpha() / 100.f );
        float beta = ( 1.0 -  alpha);
        cv::cvtColor( imgResized, imgResized, CV_GRAY2RGB );
        cv::addWeighted( imgResized, alpha, labelsResized, beta, 0.0, imgCombined);
        imgShow = imgCombined;
    } else {
        imgShow = imgResized;
    }

    cv::imshow( sliceDisplayer->getWindowName(), imgShow );
}



CliApplication::CliApplication( int argc, char *argv[] )
{
    this->argc = argc;
    this->argv = argv;
}


int CliApplication::exec()
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("images,i", po::value< std::vector<std::string> >(), "images to import")
        ("project,p", po::value< std::string >(), "project file")
        ("gpu", po::value< bool >()->implicit_value(true), "enable/disable GPU optimizations")
        ("align,a", po::value< int >()->implicit_value(1), "aligns the slices")
        ("segment,s", po::value< int >()->implicit_value(1), "segments the volume")
        ("display,d", "displays the result in a basic GUI")
        ("output,o", po::value< std::string >(), "folder path to resulting *.mrc files")
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

    if( vm.count("gpu") ) {
        segManager.setUseGPU( vm["gpu"].as< bool >() );
    }


    if( segManager.isEmpty() ) {
        std::cerr << "Empty Project (--help)" << std::endl;
        return EXIT_FAILURE;
    }

    if( vm.count("align") ) {
        for( int i = 0; i < vm["align"].as< int >(); i++ ) {
            segManager.alignSlices();
        }
    }

    if( vm.count("segment") ) {
        for( int i = 0; i < vm["segment"].as< int >(); i++ ) {
            segManager.segment();
        }
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
