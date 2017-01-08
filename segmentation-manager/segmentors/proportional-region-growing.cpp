#include <iostream>
#include <set>
#include <chrono>

#include "proportional-region-growing.h"
#include <opencv2/gpu/gpu.hpp>
#include "../point.h"

#define SEG_DEBUG 0

#ifdef __MIC__
#define my_steady_clock std::chrono::monotonic_clock
#else
#define my_steady_clock std::chrono::steady_clock
#endif

using namespace std;

bool seedComparator (Seed i, Seed j) { return (i.average<j.average); }

ProportionalRegionGrowing::ProportionalRegionGrowing(Slice slice, int minimumFeatureSize, int morphologicalSize) {
    this->img = slice.getImg();
    cv::blur( this->img, this->bluredImg, cv::Size(9,9) );

    this->seeds = slice.getSeeds();
    this->minimumFeatureSize = minimumFeatureSize;
    this->morphologicalSize = morphologicalSize;
    this->useGPU = USE_GPU_DEFAULT;

    vector<Seed> sortedSeeds = seeds;

    std::sort (sortedSeeds.begin(), sortedSeeds.end(), seedComparator);

    vector<int> chunks;
    chunks.push_back(0);

    for( size_t i = 0; i < sortedSeeds.size() - 1; i++ ) {
        int interval = sortedSeeds[i].relativeStdDev * ((sortedSeeds[i+1].average - sortedSeeds[i].average) * 1.0f / (sortedSeeds[i].relativeStdDev + sortedSeeds[i+1].relativeStdDev) * 1.0f);
        chunks.push_back( sortedSeeds[i].average + interval );
        if( SEG_DEBUG ) cout << "SeedSorted #" << i << "\tμ: " << this->seeds[i].average << "\tσ: " << this->seeds[i].relativeStdDev << "\t int: " << interval << endl;
    }

    chunks.push_back(255);

    for( size_t i = 0; i < sortedSeeds.size(); i++ ) {
        Seed s = sortedSeeds[i];
        this->intervals[s.getId()] = std::make_pair(chunks[i], chunks[i+1]);
        if( SEG_DEBUG ) cout << "-" << s.getId() << endl;
    }

    if( SEG_DEBUG ) {
        for(int interval : chunks) {
            cout << interval << endl;
        }
    }
}

ProportionalRegionGrowing::~ProportionalRegionGrowing()
{

}

cv::Mat Erode(cv::Mat masks, int size) {
    cv::Mat res;
    masks.copyTo(res);

    size = size / 2;

// #pragma omp parallel for
    for( int i = 0; i < masks.rows; i++ ) {
        for( int j = 0; j < masks.cols; j++ ) {

            bool fits = true;

            for( int y = max(0, i - size); fits && y < min(masks.rows, i + size); y++ ) {
                for( int x = max(0, j - size); fits && x < min(masks.cols, j + size); x++ ) {
                    fits = masks.at<uchar>(y, x) == masks.at<uchar>(i, j);
                }
            }

            if( !fits ) {
                res.at<uchar>(i, j) = EMPTY;
            }

        }
    }

    return res;
}

cv::Mat Dilate(cv::Mat masks, int size) {
    cv::Mat res;
    masks.copyTo(res);

    size = size / 2;
// #pragma omp parallel for
    for( int i = 0; i < masks.rows; i++ ) {
        for( int j = 0; j < masks.cols; j++ ) {

            uchar color = EMPTY;

            for( int y = max(0, i - size); y < min(masks.rows, i + size); y++ ) {
                for( int x = max(0, j - size); x < min(masks.cols, j + size); x++ ) {
                    color = min(color, masks.at<uchar>(y, x));
                }
            }

            res.at<uchar>(i, j) = color;

        }
    }

    return res;
}

bool ProportionalRegionGrowing::FindNextSeed( Seed* res, cv::Mat labels, int minSize, Point* p ) {

    for(int y = p->y; y < img.rows - minSize; y++) {
        for(int x = 0; x < img.cols - minSize; x++) {
            Point a(x, y);
            Point b(x + minSize, y + minSize);
            Region labelRegion(labels, a, b);

            if( labelRegion.isColor<uchar>(EMPTY) ) {
                Region imgRegion(this->img, a, b);

                if( imgRegion.centerOfMassIsMiddle() ) {
                    Seed s(this->img, a, b);
                    *res = s;
                    p->y = y;
                    p->x = x;
                    return true;
                }
            }
        }
    }

    return false;
}

void displayImageApagar(string title, cv::Mat img, int x = 0, int y = 100) {
    cv::imwrite("next_Seed.jpg", img);

    if( img.rows > 1000 ) {
        int newHigh = 600;
        int newWidth = img.cols * newHigh / img.rows;
        cv::resize(img, img, cv::Size(newWidth, newHigh));
    }

    cv::namedWindow(title, cv::WINDOW_AUTOSIZE);
    cv::moveWindow(title, x, y);
    cv::imshow(title, img);
}

Point ProportionalRegionGrowing::linearFill( cv::Mat& res, cv::Mat& visited, uchar seedId, Point src, bool (*pixelJudge)(int,void*), void* aditionalJudgeParams ) {

    int lFillLoc, rFillLoc;
    lFillLoc = rFillLoc = src.x;

    while( true ) {
        res.at<uchar>(src.y, lFillLoc) = seedId;
        visited.at<uchar>(src.y, lFillLoc) = 1;

        lFillLoc--;

        if( lFillLoc < 0 || res.at<uchar>(src.y, lFillLoc) != EMPTY || visited.at<uchar>(src.y, lFillLoc) || !(*pixelJudge)(bluredImg.at<uchar>(src.y, lFillLoc), aditionalJudgeParams) ) {
            break;
        }
    }

    while( true ) {
        res.at<uchar>(src.y, rFillLoc) = seedId;
        visited.at<uchar>(src.y, rFillLoc) = 1;

        rFillLoc++;

        if( rFillLoc >= img.cols || res.at<uchar>(src.y, rFillLoc) != EMPTY || visited.at<uchar>(src.y, rFillLoc) || !(*pixelJudge)(bluredImg.at<uchar>(src.y, rFillLoc), aditionalJudgeParams) ) {
            break;
        }
    }

    return Point( lFillLoc + 1, rFillLoc - 1, src.y );
}

// TODO: this solution to improve the modularity may not be the best
// instead of receiving the aditionalJudgeParams arg, it might be better
// to create an object ProportinalJudge that encapsulates it
void ProportionalRegionGrowing::RegionGrowing( cv::Mat& res, Seed seed, bool (*pixelJudge)(int,void*), void* aditionalJudgeParams ) {
    const int seedId = seed.getId();

    vector<Point> queue;
    cv::Mat visited(res.rows, res.cols, CV_8U, cv::Scalar(0));

    for( int i = seed.a.y; i < seed.b.y; i++ ) {
        if(seed.a.x) queue.push_back( linearFill( res, visited, seedId, Point(seed.a.x - 1, i), pixelJudge, aditionalJudgeParams ) );
        queue.push_back( linearFill( res, visited, seedId, Point(seed.b.x + 1, i), pixelJudge, aditionalJudgeParams ) );
    }

    for( int j = seed.a.x; j < seed.b.x; j++ ) {
        if(seed.a.y) queue.push_back( linearFill( res, visited, seedId, Point(j, seed.a.y - 1), pixelJudge, aditionalJudgeParams ) );
        queue.push_back( linearFill( res, visited, seedId, Point(j, seed.a.y + 1), pixelJudge, aditionalJudgeParams ) );
    }

    while( !queue.empty() ) {
        Point range = queue.back();
        queue.pop_back();

        for( int x = range.x; x < range.y; x++ ) {
            if( range.z > 0 && res.at<uchar>(range.z - 1, x) == EMPTY && !visited.at<uchar>(range.z - 1, x) && pixelJudge(bluredImg.at<uchar>(range.z - 1, x), aditionalJudgeParams) ) {
                queue.push_back( linearFill( res, visited, seedId, Point(x, range.z - 1), pixelJudge, aditionalJudgeParams) );
            }
        }

        for( int x = range.x; x < range.y; x++ ) {
            if( range.z + 1 < img.rows && res.at<uchar>(range.z + 1, x) == EMPTY && !visited.at<uchar>(range.z + 1, x) && pixelJudge(bluredImg.at<uchar>(range.z + 1, x), aditionalJudgeParams) ) {
                queue.push_back( linearFill( res, visited, seedId, Point(x, range.z + 1), pixelJudge, aditionalJudgeParams) );
            }
        }
    }
}

// aditionalJudgeParams[0] - inferior histogram limmit
// aditionalJudgeParams[1] - superior histogram limmit
bool proportionalJudge(int intensity, void* aditionalJudgeParams) {
    int* judgeParams = (int*) aditionalJudgeParams;
    return ( intensity >= judgeParams[0] && intensity < judgeParams[1]);
}

bool standardDeviationJudge(int intensity, void* aditionalJudgeParams) {
    int* judgeParams = (int*) aditionalJudgeParams;
    int seedAvg = judgeParams[0];
    int seedStdDev = judgeParams[1];
    int relax = 2;
    return ( intensity >= (seedAvg - relax*seedStdDev) && intensity < (seedAvg + relax*seedStdDev) );
}

void ProportionalRegionGrowing::InitialConquer(cv::Mat& res) {
    int proportionalSeedIntervals[2];

    for( Seed seed : this->seeds ) {
        std::pair<int, int> localInterval = this->intervals[seed.getId()];
        proportionalSeedIntervals[0] = localInterval.first;    // inferior histogram limmit
        proportionalSeedIntervals[1] = localInterval.second;  // superior histogram limmit

        this->RegionGrowing( res, seed, proportionalJudge, proportionalSeedIntervals );
    }
}

void ProportionalRegionGrowing::AutomaticConquer(cv::Mat& res) {
    Seed nextSeed;
//    int mm = 3;

    Point start(0, 0);
    while( this->FindNextSeed( &nextSeed, res, this->minimumFeatureSize, &start ) ){
//        if( !mm-- ) break;
//        cv::Mat imgWithNewSeed;
//        cv::cvtColor(this->img, imgWithNewSeed, cv::COLOR_GRAY2BGR);
//        nextSeed.draw(imgWithNewSeed);

//        displayImageApagar("New Seed", imgWithNewSeed);
//        cv::waitKey();


        Seed* similarSeed = nextSeed.getSimilarSeed( this->seeds );
        if( similarSeed ) {
            nextSeed.setId( similarSeed->getId() );

            int proportionalSeedIntervals[2];
            std::pair<int, int> localInterval = this->intervals[nextSeed.getId()];
            proportionalSeedIntervals[0] = localInterval.first;     // inferior histogram limmit
            proportionalSeedIntervals[1] = localInterval.second;    // superior histogram limmit

            if( SEG_DEBUG ) cout << "Similar Seed: " << nextSeed.getId() << endl;

            this->RegionGrowing( res, nextSeed, proportionalJudge, proportionalSeedIntervals);
        } else {
            // TODO: find out and apply convinient RegionGrowing
            // or endless FindNextSeed loop
            if( SEG_DEBUG ) cout << "Degenerated Seed Found" << endl;
            if( SEG_DEBUG ) cout << nextSeed << endl;

//            Seed similarByAvg = nextSeed.getMoreSimilarSeedByAvg( this->seeds );
//            Seed similarByStdDev = nextSeed.getMoreSimilarSeedByStdDev( this->seeds );

            float grade;
            Seed* bestGradedSeed = nextSeed.getBestGradedSeed(this->seeds, res, &grade);
            if( SEG_DEBUG ) cout << "Similar: \t" << bestGradedSeed->getId() << "\tGrade: " << grade << endl;

//            {
//                cv::Mat imgWithNewSeed;
//                cv::cvtColor(this->img, imgWithNewSeed, cv::COLOR_GRAY2BGR);
//                nextSeed.draw(imgWithNewSeed);

//                displayImageApagar("New Seed", imgWithNewSeed);
//                cv::waitKey();
//            }

//            cout << "Similar By Avg: \t" << similarByAvg.id << endl;

            nextSeed.setId( bestGradedSeed->getId() );

            int seedAvgAndStdDev[2];
            seedAvgAndStdDev[0] = nextSeed.average;
            seedAvgAndStdDev[1] = nextSeed.relativeStdDev;
            this->RegionGrowing( res, nextSeed, standardDeviationJudge, seedAvgAndStdDev );
        }

    }
}

void ProportionalRegionGrowing::MorphologicalFiltering(cv::Mat& res) {
    int erosion_size = this->morphologicalSize / 2;
    cv::Mat element = cv::getStructuringElement( cv::MORPH_ELLIPSE,
        cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
        cv::Point(erosion_size, erosion_size)
    );

    if( this->useGPU ) {
        std::cout << "\tUsing GPU" << std::endl;
        //erodeAndDilate_GPU(&(res.at<uchar>(0)), this->morphologicalSize, res.cols, res.rows);
        cv::gpu::GpuMat src_gpu, dst_gpu;
        cv::gpu::Stream stream;
        stream.enqueueUpload(res, src_gpu);
        cv::gpu::morphologyEx(src_gpu,dst_gpu, cv::MORPH_OPEN, element);
        stream.enqueueDownload(dst_gpu, res);
    } else {
        std::cout << "\tUsing CPU" << std::endl;
        cv::morphologyEx(res, res, cv::MORPH_OPEN, element);
        // res = Erode(res, this->morphologicalSize);
        // res = Dilate(res, this->morphologicalSize);
    }
}

void ProportionalRegionGrowing::FillTinyHoles(cv::Mat& res) {
    int count = 0;
    for( int i = 0; i < res.rows; i++ ) {
        for( int j = 0; j < res.cols; j++ ) {
            if( res.at<uchar>(i, j) == EMPTY ) {
                // try to find out which phases are accessible through EMPTY pixels

                vector<Point> queue;
                vector<Point> toPaint;
                set<uchar> accessiblePhasesID;
                int intensitySum = 0;

                cv::Mat visited(res.rows, res.cols, CV_8U);
                visited = cv::Scalar( 0 );

                queue.push_back(Point(j, i));
                queue.push_back(Point(j - 1, i));
                queue.push_back(Point(j + 1, i));
                queue.push_back(Point(j, i - 1));
                queue.push_back(Point(j, i + 1));

                while( !queue.empty() ) {
                    Point p = queue.back();
                    queue.pop_back();

                    if( p.y >= 0 && p.x >= 0 && p.y < res.rows && p.x < res.cols && !visited.at<uchar>(p.y, p.x) ) {
                        uchar currentPhase = res.at<uchar>(p.y, p.x);
                        if( currentPhase != EMPTY ) {
                            accessiblePhasesID.insert( currentPhase );
                        } else {
                            toPaint.push_back(p);
                            intensitySum += this->img.at<uchar>(p.y, p.x);

                            queue.push_back(Point(p.x - 1, p.y));
                            queue.push_back(Point(p.x + 1, p.y));
                            queue.push_back(Point(p.x, p.y - 1));
                            queue.push_back(Point(p.x, p.y + 1));
                        }

                        visited.at<uchar>(p.y, p.x) = 1;
                    }
                }

//                int avg = intensitySum / toPaint.size();
//                int bestAvgDiff = INT_MAX;
                int bestSeedID = INT_MAX;
                for(Seed s : this->seeds) {
                    if( accessiblePhasesID.find( s.getId() ) != accessiblePhasesID.end() ) {
//                        cout << "#" << s.id << endl;
//                        int localDiff = abs(s.average - avg);
//                        if( localDiff < bestAvgDiff ) {
//                            bestAvgDiff = localDiff;
                        bestSeedID = s.getId();
                        break;
//                        }
                    }
                }

                for(Point p : toPaint) {
                    count++;
                    res.at<uchar>(p.y, p.x) = bestSeedID;
                }
            }
        }
    }

    int total = res.rows * res.cols;
    if( SEG_DEBUG ) cout << "Empty: " << count << " of " << total << " (" << (count*1.0f / total) * 100 << "%)" << endl;
}

void ProportionalRegionGrowing::setUseGPU(bool value) {
    this->useGPU = value;
}

cv::Mat ProportionalRegionGrowing::Apply() {
    cv::Mat res(img.rows, img.cols, CV_8U);
    res = cv::Scalar( EMPTY ); // start with no material

    my_steady_clock::time_point initialConquerBeginTime = my_steady_clock::now();
    this->InitialConquer(res);
    my_steady_clock::time_point initialConquerEndTime = my_steady_clock::now();

    my_steady_clock::time_point automaticConquerBeginTime = my_steady_clock::now();
    this->AutomaticConquer(res);
    my_steady_clock::time_point automaticConquerEndTime = my_steady_clock::now();

    my_steady_clock::time_point morphologicalFilteringBeginTime = my_steady_clock::now();
    if( this->morphologicalSize > 1 ) {
        this->MorphologicalFiltering(res);
    }
    my_steady_clock::time_point morphologicalFilteringEndTime = my_steady_clock::now();

    my_steady_clock::time_point fillTinyGapsBeginTime = my_steady_clock::now();
    this->FillTinyHoles(res);
    my_steady_clock::time_point fillTinyGapsEndTime = my_steady_clock::now();


    std::cout << "Initial Conquer\t" << std::chrono::duration_cast<std::chrono::milliseconds>(initialConquerEndTime - initialConquerBeginTime).count() << std::endl;
    std::cout << "Automatic Conquer\t" << std::chrono::duration_cast<std::chrono::milliseconds>(automaticConquerEndTime - automaticConquerBeginTime).count() << std::endl;
    std::cout << "Morphological Filtering\t" << std::chrono::duration_cast<std::chrono::milliseconds>(morphologicalFilteringEndTime - morphologicalFilteringBeginTime).count() << std::endl;
    std::cout << "Fill Tiny Holes\t" << std::chrono::duration_cast<std::chrono::milliseconds>(fillTinyGapsEndTime - fillTinyGapsBeginTime).count() << std::endl;

    return res;
}
