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
    if( segManager.isEmpty() ) {
        return 0;
    }


    this->definePhasisSeeds(0);

    cv::vector<Slice>& slices = segManager.getSlices();

    Slice* slice = &slices[0];

    std::vector<Seed>& seeds = slice->getSeeds();
    slice->setMinimumFeatureSize(15);

    segManager.apply(0);


    for( int i = 0; i < slices.size(); i++ ) {
        Slice& s = slices[i];

        cv::Mat res = colorizeLabels(s.getSegmentationResult(), seeds);

         // cv::imwrite( "/Users/Paulo/Projetos/Tese/TomSeg/datasets/ROI/result/sliceRes_" + std::to_string(i+1) + ".jpg", res);

        // displayImage("Slice " + std::to_string(i+1) + " Subtracted", res, 650);
    }


    cv::Mat imgWithSeeds;
    cv::cvtColor(slice->getImg(), imgWithSeeds, cv::COLOR_GRAY2BGR);
    for( Seed s : seeds ) {
        s.draw(imgWithSeeds);
    }

    displayImage("Original With Seeds", imgWithSeeds);


    cv::waitKey(0);



    return 1;
}
