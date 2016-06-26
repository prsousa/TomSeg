#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;


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

    // code here

    displayImage(img);

    return 0;
}
