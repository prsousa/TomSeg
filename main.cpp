#include <iostream>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "seed.h"
#include "point.h"

using namespace std;

cv::Mat segmentationPixelByPixel(cv::Mat img, vector<Seed> seeds) {
    cv::Mat res(img.rows, img.cols, CV_8U);

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

            res.at<uchar>(i, j) = bestSeed_id;
        }
    }

    return res;
}

cv::Mat segmentation(cv::Mat img, vector<Seed> seeds) {
    cv::Mat res(img.rows, img.cols, CV_8U);
    res = cv::Scalar( seeds.size() ); // start with no material

    for(int k = 0; k < seeds.size(); k++) {
        Seed seed = seeds[k];

        vector<Point> queue;
        cv::Mat visited(img.rows, img.cols, CV_8U);

        for( int i = seed.a.y; i < seed.b.y; i++ ) {
            queue.push_back(Point(seed.a.x - 1, i));
            queue.push_back(Point(seed.b.x + 1, i));
        }

        for( int j = seed.a.x; j < seed.b.x; j++ ) {
            queue.push_back(Point(j, seed.a.y - 1));
            queue.push_back(Point(j, seed.b.y + 1));
        }

        while( !queue.empty() ) {
            Point p = queue.back();
            queue.pop_back();

            if( p.y >= 0 && p.x >= 0 && p.y < img.rows && p.x < img.cols && !visited.at<uchar>(p.y, p.x) ) {
                int bluredIntensity = 0;
                int n = 0;
                {
                    int blurSiz = 9 / 2;

                    for(int b = max(p.y - blurSiz, 0); b <= min(p.y + blurSiz, img.rows); b++) {
                        for(int a = max(p.x - blurSiz, 0); a <= min(p.x + blurSiz, img.cols); a++) {
                            bluredIntensity += (int) img.at<uchar>(b, a);
                            n++;
                        }
                    }
                }

                bluredIntensity = bluredIntensity / n;

                // also tried: use minium difference -> diff = min( "blured" value, original one)
                uchar diff = abs(bluredIntensity - seed.average);

                if( diff <= 2 * seed.stdDev ) {
                    res.at<uchar>(p.y, p.x) = k;

                    queue.push_back(Point(p.x + 1, p.y));
                    queue.push_back(Point(p.x - 1, p.y));
                    queue.push_back(Point(p.x, p.y - 1));
                    queue.push_back(Point(p.x, p.y + 1));

                    // corners (8-way)
                    queue.push_back(Point(p.x - 1, p.y - 1));
                    queue.push_back(Point(p.x - 1, p.y + 1));
                    queue.push_back(Point(p.x + 1, p.y - 1));
                    queue.push_back(Point(p.x + 1, p.y + 1));
                }

                visited.at<uchar>(p.y, p.x) = 1;
            }
        }
    }

    return res;
}

// converts a matrix of labels into a colored image
cv::Mat colorizeLabels(cv::Mat labels, vector<Seed> seeds) {
    cv::Mat res(labels.rows, labels.cols, CV_8UC3);

    for(int i = 0; i < labels.rows; i++) {
        for(int j = 0; j < labels.cols; j++) {
            int label = labels.at<uchar>(i, j);
            res.at<cv::Vec3b>(i, j)[0] = seeds[label].color[0];
            res.at<cv::Vec3b>(i, j)[1] = seeds[label].color[1];
            res.at<cv::Vec3b>(i, j)[2] = seeds[label].color[2];
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
//     cv::GaussianBlur( imgs[0], imgs[0], cv::Size( 9, 9 ), 0, 0 );
    cv::Mat labels = segmentation(imgs[0], seeds);
    cv::Mat res = colorizeLabels(labels, seeds);

    cv::Mat imgWithSeeds;
    cv::cvtColor(imgs[0], imgWithSeeds, cv::COLOR_GRAY2BGR);
    for( int i = 0; i < seeds.size(); i++) {
        Seed s = seeds[i];
        s.draw(imgWithSeeds);
        cout << "Seed #" << i << "\tμ: " << s.average << "\tσ: " << s.stdDev << endl;
    }

    displayImage("Original With Seeds", imgWithSeeds);
    displayImage("Result", res, 650);

    cv::waitKey(0);

    return 0;
}
