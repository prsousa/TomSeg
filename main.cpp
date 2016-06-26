#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "seed.h"
#include "point.h"

using namespace std;


void definePhasisSeeds(cv::Mat img, vector<Seed> &seeds) {
    seeds.push_back(Seed(img, Point(300, 550), Point(350, 600)));
    seeds.push_back(Seed(img, Point(6, 529), Point(25, 543)));
    seeds.push_back(Seed(img, Point(98, 455), Point(150, 507)));
    seeds.push_back(Seed(img, Point(308, 259), Point(386, 327)));
    seeds.push_back(Seed(img, Point(290, 23), Point(390, 93)));
}

void displayImage(cv::Mat img) {
    if( img.rows > 1000 ) {
        cv::resize(img, img, cv::Size(img.cols / 2, img.rows / 2));
    }

    cv::namedWindow("Segments", cv::WINDOW_AUTOSIZE);
    cv::imshow("Segments", img);
    cv::waitKey(0);
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cerr << "Error: Image not specified!" << endl;
        return EXIT_FAILURE;
    }

    string filename = argv[1];
    cv::Mat img = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
    if (img.empty()) {
        cerr << "Error: Image cannot be loaded!\n" << endl;
        return EXIT_FAILURE;
    }

    vector<Seed> seeds;
    definePhasisSeeds(img, seeds);


    // segmentation code goes here

    cv::Mat res;
    cv::cvtColor(img, res, cv::COLOR_GRAY2BGR);
    for( Seed s : seeds) {
        s.draw(res);
        cout << s.average << "\t" << s.stdDev << endl;
    }

    displayImage(res);

    return 0;
}
