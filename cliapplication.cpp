#include "cliapplication.h"
#include <iostream>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

CliApplication::CliApplication( int argc, char *argv[] )
{
    std::vector<std::string> filenames;

    for( int i = 1; i < argc; i++ ) {
        filenames.push_back(argv[i]);
    }

    segManager.setSlices(filenames);
}

void CliApplication::definePhasisSeeds( int sliceNumber )
{
    Slice* slice = segManager.getSlice(sliceNumber);

    std::string seedsFileName = slice->getFilename() + ".seeds";

    std::fstream file;
    int sA_x, sA_y, sB_x, sB_y;

    file.open(seedsFileName, std::fstream::in);

    if (!file) {
        std::cerr << "Error: Seeds file cannot be loaded!" << std::endl;
        return;
    }

    std::vector<Seed> seeds;
    int i = 0;

    while (file >> sA_x >> sA_y >> sB_x >> sB_y) {
        seeds.push_back(Seed(slice->getImg(), i++, Point(sA_x, sA_y), Point(sB_x, sB_y)));
    }

    slice->setSeeds(seeds);
}

void displayImage(std::string title, cv::Mat img, int x = 0, int y = 100) {
    if( img.rows > 1000 ) {
        int newHigh = 600;
        int newWidth = img.cols * newHigh / img.rows;
        cv::resize(img, img, cv::Size(newWidth, newHigh));
    }

    cv::namedWindow(title, cv::WINDOW_AUTOSIZE);
    cv::moveWindow(title, x, y);
    cv::imshow(title, img);
}

cv::Mat CliApplication::colorizeLabels(cv::Mat labels, std::vector<Seed> seeds)
{
    cv::Mat res(labels.rows, labels.cols, CV_8UC3);
    res = cv::Scalar( 0 );

    for(int i = 0; i < labels.rows; i++) {
        for(int j = 0; j < labels.cols; j++) {
            int label = labels.at<uchar>(i, j);
            if(label != EMPTY) {
                res.at<cv::Vec3b>(i, j)[0] = seeds[label].c_r;
                res.at<cv::Vec3b>(i, j)[1] = seeds[label].c_g;
                res.at<cv::Vec3b>(i, j)[2] = seeds[label].c_b;
            }
        }
    }

    return res;
}

int CliApplication::exec()
{
    if( segManager.isEmpty() ) {
        return 0;
    }


    this->definePhasisSeeds(0);

    Slice* slice = segManager.getSlice(0);
    cv::vector<Seed>& seeds = slice->getSeeds();
    slice->setMinimumFeatureSize(15);

    segManager.apply(0);


    cv::Mat res = colorizeLabels(slice->getSegmentationResult(), seeds);

    cv::Mat imgWithSeeds;
    cv::cvtColor(slice->getImg(), imgWithSeeds, cv::COLOR_GRAY2BGR);
    for( Seed s : seeds ) {
        s.draw(imgWithSeeds);
    }

    displayImage("Original With Seeds", imgWithSeeds);
    displayImage("Result", res, 650);

    cv::waitKey(0);



    return 1;
}
