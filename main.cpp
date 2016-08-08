#include <iostream>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "seed.h"
#include "point.h"
#include "segmentors/segmenter.h"
#include "segmentors/region-growing.h"
#include "segmentors/proportional-region-growing.h"
#include "segmentors/pixel-by-pixel.h"
#include "segmentors/proportional-pixel-by-pixel.h"
#include "segmentors/grided-region-growing.h"

using namespace std;

// converts a matrix of labels into a colored image
cv::Mat colorizeLabels(cv::Mat labels, vector<Seed> seeds) {
    cv::Mat res(labels.rows, labels.cols, CV_8UC3);
    res = cv::Scalar( 0 );

    for(int i = 0; i < labels.rows; i++) {
        for(int j = 0; j < labels.cols; j++) {
            int label = labels.at<uchar>(i, j);
            if(label != EMPTY) {
                res.at<cv::Vec3b>(i, j)[0] = seeds[label].color[0];
                res.at<cv::Vec3b>(i, j)[1] = seeds[label].color[1];
                res.at<cv::Vec3b>(i, j)[2] = seeds[label].color[2];
            }
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

void displayImage(string title, cv::Mat img, int x = 0, int y = 100) {
    if( img.rows > 1000 ) {
        int newHigh = 600;
        int newWidth = img.cols * newHigh / img.rows;
        cv::resize(img, img, cv::Size(newWidth, newHigh));
    }

    cv::namedWindow(title, cv::WINDOW_AUTOSIZE);
    cv::moveWindow(title, x, y);
    cv::setMouseCallback(title, mouseHandlerFunc, NULL);
    cv::imshow(title, img);
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cerr << "Error: Image not specified!" << endl;
        return EXIT_FAILURE;
    }

    vector<cv::Mat> imgs(10);

    for(int i = 0; i < argc - 1; i++) {
        string imagename = argv[i+1];
        imgs[i] = cv::imread(imagename, CV_LOAD_IMAGE_GRAYSCALE);
        if (imgs[i].empty()) {
            cerr << "Error: Image cannot be loaded!\n" << endl;
            return EXIT_FAILURE;
        }
    }

    string seedname = string(argv[1]) + ".seeds";

    vector<Seed> seeds;
    definePhasisSeeds(seedname, imgs[0], seeds);

    // segmentation code goes here
    Segmenter* segmenter = new ProportionalRegionGrowing(imgs[0], seeds);
    cv::Mat labels = segmenter->Apply();

    cv::Mat res = colorizeLabels(labels, seeds);

    cv::Mat imgWithSeeds;
    cv::cvtColor(imgs[0], imgWithSeeds, cv::COLOR_GRAY2BGR);
    for( int i = 0; i < seeds.size(); i++) {
        Seed s = seeds[i];
        s.draw(imgWithSeeds);
        cout << "Seed #" << i << "\tμ: " << s.average << "\tσ: " << s.relativeStdDev << endl;
    }

    imwrite("seeded.jpg", imgWithSeeds);
    imwrite("result.jpg", res);

    displayImage("Original With Seeds", imgWithSeeds);
    displayImage("Result", res, 650);

    cv::waitKey(0);

    return 0;
}
