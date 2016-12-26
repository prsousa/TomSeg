#include <chrono>
#include <iostream>

#include "segmentationmanager.h"

#include "segmentors/segmenter.h"
#include "segmentors/proportional-region-growing.h"
#include "differentiators/differentiator.h"
#include "preprocessors/aligner.h"
#include "exporter.h"
#include "importer.h"
#include "seedpropagater.h"

using namespace std;

SegmentationManager::SegmentationManager()
{
    minimumFeatureSize = 15;
    morphologicalSize = 15;
    xLen = yLen = zLen = 1.f;
    useGPU = USE_GPU_DEFAULT;
}

SegmentationManager::SegmentationManager(string projectPath) : SegmentationManager()
{
    std::chrono::steady_clock::time_point beginImporting = std::chrono::steady_clock::now();

    Importer importer( this );
    importer.importProject( projectPath );

    this->projectPath = projectPath;

    std::chrono::steady_clock::time_point endImporting = std::chrono::steady_clock::now();
    std::cout << "ImportProject:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(endImporting - beginImporting).count() << std::endl;

}

void SegmentationManager::setSlices(vector<string> &filenames)
{
    std::chrono::steady_clock::time_point beginFileLoading = std::chrono::steady_clock::now();
    slices.clear();

    vector<string>::iterator i;
    for( i = filenames.begin(); i != filenames.end(); i++ ) {
        string imagename = *i;
        this->addSlice( Slice(imagename) );
    }

    std::chrono::steady_clock::time_point endFileLoading = std::chrono::steady_clock::now();
    std::cout << "LoadTime:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(endFileLoading - beginFileLoading).count() << std::endl;
}

void SegmentationManager::addSlice(Slice slice)
{
    if( !slice.getImg().empty() ) {
        slices.push_back( slice );
    }
}

void SegmentationManager::setSliceSeeds(size_t sliceNumber, const std::vector<Seed>& seeds)
{
    Slice& slice = this->slices[sliceNumber];
    slice.setSeeds( seeds );
}

void SegmentationManager::alignSlices()
{
    Aligner aligner(this->slices.begin(), this->slices.end());
    aligner.apply();
}

void SegmentationManager::alignSlices( size_t masterSliceNumber, Point a, size_t width, size_t height, int maxDeltaX, int maxDeltaY )
{
    if( masterSliceNumber < this->slices.size() ) {
        Slice& masterSlice = this->slices[masterSliceNumber];

        std::chrono::steady_clock::time_point beginAlign = std::chrono::steady_clock::now();

        Aligner aligner(this->slices.begin(), this->slices.end(), maxDeltaX, maxDeltaY);
        aligner.apply( masterSlice.getImg(), a, width, height );

        std::chrono::steady_clock::time_point endAlign = std::chrono::steady_clock::now();
        std::cout << "AlignSlices:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(endAlign - beginAlign).count() << std::endl;
    }
}

void SegmentationManager::cropSlices(size_t firstSlice, size_t lastSlice, Point a, size_t width, size_t height)
{
    firstSlice--;
    lastSlice--;

    vector<Slice>::iterator cutStrt = slices.begin();
    vector<Slice>::iterator cutStop = cutStrt + firstSlice;

    if( firstSlice > 0 ) {
        slices.erase(slices.begin(), slices.begin() + firstSlice);
    }

    cutStrt = slices.begin() + (lastSlice - firstSlice + 1);
    cutStop = slices.end();

    if( cutStrt < cutStop ) {
        slices.erase(cutStrt, cutStop);
    }

    for( Slice& slice : slices ) {
        slice.crop(a, width, height);
    }
}

void SegmentationManager::resetResults()
{
    for( Slice& slice : slices ) {
        slice.resetSegmentationResult();
    }
}

void SegmentationManager::resetSeeds()
{
    this->resetSeeds(0, slices.size());
}

void SegmentationManager::resetSeeds(int sliceIndex)
{
    this->resetSeeds(sliceIndex, sliceIndex + 1);
}

void SegmentationManager::resetSeeds(int start, int end)
{
    std::vector<Slice>::iterator it = slices.begin() + start;
    std::vector<Slice>::iterator stop = slices.begin() + end;

    for( ; it != stop; it++ ) {
        Slice& slice = *it;
        slice.resetSeeds();
    }
}

void SegmentationManager::resetSeedsFrom(int start)
{
    this->resetSeeds( start, slices.size() );
}

void SegmentationManager::exportResult(string path)
{
    this->exportResult(path, 1, this->size());
}

void SegmentationManager::exportResult(string path, size_t firstSlice, size_t lastSlice)
{
    if (lastSlice < firstSlice ) return;

    Exporter exporter( this, firstSlice, lastSlice );
    exporter.exportResult(path);
}

void SegmentationManager::exportSlicesImages(string path)
{
    this->exportSlicesImages(path, 1, this->size());
}

void SegmentationManager::exportSlicesImages(string path, size_t firstSlice, size_t lastSlice)
{
    if (lastSlice < firstSlice ) return;

    Exporter exporter( this, firstSlice, lastSlice );
    exporter.exportSlicesImages(path);
}

void SegmentationManager::exportProject(string path)
{
    this->exportProject(path, 1, this->size());
}

void SegmentationManager::exportProject(string path, size_t firstSlice, size_t lastSlice)
{
    if (lastSlice < firstSlice ) return;

    Exporter exporter( this, firstSlice, lastSlice );
    exporter.exportProject( path );

    this->projectPath = path;
}

std::vector<Slice> &SegmentationManager::getSlices()
{
    return this->slices;
}

Slice* SegmentationManager::getSlice(size_t sliceNumber)
{
    return &(this->slices[sliceNumber]);
}

void SegmentationManager::propagateSeeds(size_t sliceNumber, size_t stride)
{
    if( sliceNumber < this->slices.size() ) {
        std::chrono::steady_clock::time_point beginPropagate = std::chrono::steady_clock::now();

        Slice& masterSlice = this->slices[sliceNumber];
        if( masterSlice.seedsNumber() >= 2 ) {
            SeedPropagater seedPropagater( this->slices.begin() + sliceNumber, this->slices.end() );
            seedPropagater.propagate( masterSlice.getSeeds(), stride );
        }

        std::chrono::steady_clock::time_point endPropagate = std::chrono::steady_clock::now();
        std::cout << "PropagateSeeds:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(endPropagate - beginPropagate).count() << std::endl;
    }
}

void SegmentationManager::segment()
{
    std::chrono::steady_clock::time_point beginSegmentation = std::chrono::steady_clock::now();

    std::vector<int> slicesWithSeedsIndex;
    {
        int i = 0;
        for( Slice& slice : this->slices ) {
            if( slice.seedsNumber() >= 2 ) {
                slicesWithSeedsIndex.push_back(i);
            }
            i++;
        }
    }

    slicesWithSeedsIndex.push_back( this->slices.size() ); // "hack" to make intervals work
    size_t slicesWithSeedsIndexSize = slicesWithSeedsIndex.size();

#pragma omp parallel for shared(slicesWithSeedsIndexSize, slicesWithSeedsIndex, std::cout) default(none) schedule(guided)
    for( size_t i = 0; i < slicesWithSeedsIndexSize - 1; i++ ) {
        int firstSliceIndex = slicesWithSeedsIndex[i];
        int lastSliceIndex = slicesWithSeedsIndex[i + 1];

        Slice& slice = this->slices[ firstSliceIndex ];

        std::chrono::steady_clock::time_point beginSegmentationTime = std::chrono::steady_clock::now();

        ProportionalRegionGrowing segmenter(slice, this->minimumFeatureSize, this->morphologicalSize);
        segmenter.setUseGPU( this->useGPU );
        cv::Mat res = segmenter.Apply();
        slice.setSegmentationResult(res);

        std::chrono::steady_clock::time_point endSegmentationTime = std::chrono::steady_clock::now();
        std::cout << "SegTime:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(endSegmentationTime - beginSegmentationTime).count() << std::endl << std::endl;

        // Apply differences to slices in between
        Differentiator dif(this->slices.begin() + firstSliceIndex, this->slices.begin() + lastSliceIndex);
        dif.apply();
    }

    std::chrono::steady_clock::time_point endSegmentation = std::chrono::steady_clock::now();
    std::cout << "Segmentation Time:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(endSegmentation - beginSegmentation).count() << std::endl << std::endl;
}

bool SegmentationManager::isEmpty()
{
    return this->slices.empty();
}

size_t SegmentationManager::size()
{
    return slices.size();
}

float SegmentationManager::getXLen() const
{
    return xLen;
}

void SegmentationManager::setXLen(float value)
{
    xLen = value;
}

float SegmentationManager::getYLen() const
{
    return yLen;
}

void SegmentationManager::setYLen(float value)
{
    yLen = value;
}

float SegmentationManager::getZLen() const
{
    return zLen;
}

void SegmentationManager::setZLen(float value)
{
    zLen = value;
}

void SegmentationManager::setMinimumFeatureSize(int value)
{
    minimumFeatureSize = value;
}

int SegmentationManager::getMinimumFeatureSize() const
{
    return minimumFeatureSize;
}

int SegmentationManager::getMorphologicalSize() const
{
    return morphologicalSize;
}

void SegmentationManager::setMorphologicalSize(int value)
{
    morphologicalSize = value;
}

bool SegmentationManager::getUseGPU() const
{
    return useGPU;
}

void SegmentationManager::setUseGPU(bool value)
{
    useGPU = value;
}

string SegmentationManager::getProjectPath() const
{
    return projectPath;
}

string SegmentationManager::getProjectFilename() const
{
    int idx = 0;
    for( int i = 0; projectPath[i]; i++ ) {
        if( projectPath[i] == '/' ) {
            idx = i;
        }
    }

    return &projectPath[idx + 1];
}

string SegmentationManager::getProjectFolderPath() const
{
    string res = projectPath;
    int idx = 0;
    for( int i = 0; projectPath[i]; i++ ) {
        if( projectPath[i] == '/' ||  projectPath[i] == '\\') {
            idx = i;
        }
    }
    res[idx + 1] = 0;
    return res;
}
