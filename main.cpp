#include <iostream>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "seed.h"
#include "point.h"

using namespace std;

cv::Mat segmentation(cv::Mat img, vector<Seed> seeds) {
    cv::Mat res(img.rows, img.cols, CV_8UC3);

    for(int i = 0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {

            uchar bestSeed_id = 0;
            uchar bestSeed_diff = 255;

            for(int k = 0; k < seeds.size(); k++) {
                uchar pixelIntensity = img.at<uchar>(i, j);
                uchar diff = abs(pixelIntensity - seeds[k].average);

                if( diff < bestSeed_diff ) {
                    bestSeed_diff = diff;
                    bestSeed_id = k;
                }
            }

            res.at<cv::Vec3b>(i, j)[0] = seeds[bestSeed_id].color[0];
            res.at<cv::Vec3b>(i, j)[1] = seeds[bestSeed_id].color[1];
            res.at<cv::Vec3b>(i, j)[2] = seeds[bestSeed_id].color[2];
        }
    }

    return res;
}

void definePhasisSeeds(string filename, cv::Mat img, vector<Seed> &seeds) {
    std::fstream file;
    int sA_x, sA_y, sB_x, sB_y;

    file.open(filename, std::fstream::in);

    if (!file) {
        cerr << "Error: Seeds file cannot be loaded!" << endl;
        return;
    }

    while (file >> sA_x >> sA_y >> sB_x >> sB_y) {
        seeds.push_back(Seed(img, Point(sA_x, sA_y), Point(sB_x, sB_y)));
    }
}

void mouseHandlerFunc(int event, int x, int y, int flags, void* userdata) {
    switch (event) {
    case cv::EVENT_LBUTTONDOWN:
        cout << "(MOUSE) " << "x: " << x << "\ty: " << y << endl;
        break;
    default:
        break;
    }
}

void displayImage(string title, cv::Mat img) {
    if( img.rows > 1000 ) {
        cv::resize(img, img, cv::Size(img.cols / 2, img.rows / 2));
    }

    cv::namedWindow(title, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(title, mouseHandlerFunc, NULL);
    cv::imshow(title, img);
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cerr << "Error: Image not specified!" << endl;
        return EXIT_FAILURE;
    }

    string imagename = argv[1];
    cv::Mat img = cv::imread(imagename, CV_LOAD_IMAGE_GRAYSCALE);
    if (img.empty()) {
        cerr << "Error: Image cannot be loaded!\n" << endl;
        return EXIT_FAILURE;
    }

    vector<Seed> seeds;
    definePhasisSeeds(imagename + ".seeds", img, seeds);

    // segmentation code goes here
    cv::Mat res = segmentation(img, seeds);


    cv::Mat imgWithSeeds;
    cv::cvtColor(img, imgWithSeeds, cv::COLOR_GRAY2BGR);
    for( int i = 0; i < seeds.size(); i++) {
        Seed s = seeds[i];
        s.draw(imgWithSeeds);
        cout << s.average << "\t" << s.stdDev << endl;
    }

    displayImage("Original With Seeds", imgWithSeeds);
    displayImage("Result", res);

    cv::waitKey(0);

    return 0;
}
