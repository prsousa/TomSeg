#include <cuda.h>
#include <cuda_runtime.h>

#define CUDA_DEBUG 0

#if CUDA_DEBUG
#include <stdio.h>
#endif

__global__
void erode_GPU( const unsigned char *labels, unsigned char *res, int size, int width, int height ) {
    int id = blockIdx.x * blockDim.x + threadIdx.x;
    if( id > width * height ) return;

    int i = id / width;
    int j = id % width;

    int color = labels[id];

    size = size / 2;

    bool fits = true;

    for( int y = max(0, i - size); fits && y < min(height, i + size); y++ ) {
        for( int x = max(0, j - size); fits && x < min(width, j + size); x++ ) {
            fits = labels[y * width + x] == color;
        }
    }

    if( !fits ) {
        res[id] = 255;
    } else {
        res[id] = color;
    }
}

__global__
void dilate_GPU( const unsigned char *labels, unsigned char *res, int size, int width, int height ) {
    int id = blockIdx.x * blockDim.x + threadIdx.x;
    if( id > width * height ) return;

    int i = id / width;
    int j = id % width;

    int color = 255;

    size = size / 2;

    for( int y = max(0, i - size); y < min(height, i + size); y++ ) {
        for( int x = max(0, j - size); x < min(width, j + size); x++ ) {
            color = min(color, labels[y * width + x]);
        }
    }

    res[id] = color;
}

void erodeAndDilate_GPU( unsigned char* labels, int size, int width, int height ) {
    unsigned char  *labels_gpu, *res;
    int sizeLabels_bytes = width * height * int(sizeof(unsigned char));

#if CUDA_DEBUG
    float millisecondsSending = 0;
    float millisecondsKernel = 0;
    float millisecondsReceiving = 0;
    float milliseconds = 0;
    cudaEvent_t start, stop, startSending, stopSending, startKernel, stopKernel, startReceiving, stopReceiving;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventCreate(&startSending);
    cudaEventCreate(&stopSending);
    cudaEventCreate(&startKernel);
    cudaEventCreate(&stopKernel);
    cudaEventCreate(&startReceiving);
    cudaEventCreate(&stopReceiving);

    cudaEventRecord(start);
#endif
    cudaMalloc((void**) &labels_gpu, sizeLabels_bytes);
    cudaMalloc((void**) &res, sizeLabels_bytes);
#if CUDA_DEBUG
    cudaEventRecord(startSending);
#endif
    cudaMemcpy(labels_gpu, labels, sizeLabels_bytes, cudaMemcpyHostToDevice);
#if CUDA_DEBUG
    cudaEventRecord(stopSending);
    cudaEventSynchronize(stopSending);
    cudaEventElapsedTime(&millisecondsSending, startSending, stopSending);

    cudaEventRecord(startKernel);
#endif
    size_t numThreads = 512;
    size_t numBloks = (width*height) / numThreads;
    erode_GPU<<<numBloks, numThreads>>>(labels_gpu, res, size, width, height);
    dilate_GPU<<<numBloks, numThreads>>>(res, labels_gpu, size, width, height);
#if CUDA_DEBUG
    cudaEventRecord(stopKernel);
    cudaEventSynchronize(stopKernel);
    cudaEventElapsedTime(&millisecondsKernel, startKernel, stopKernel);

    cudaEventRecord(startReceiving);
#endif
    cudaMemcpy(labels, labels_gpu, sizeLabels_bytes, cudaMemcpyDeviceToHost);
#if CUDA_DEBUG
    cudaEventRecord(stopReceiving);
    cudaEventSynchronize(stopReceiving);
    cudaEventElapsedTime(&millisecondsReceiving, startReceiving, stopReceiving);
#endif
    cudaFree(labels_gpu);
    cudaFree(res);
#if CUDA_DEBUG
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&milliseconds, start, stop);

    printf("Sending Time:\t%f ms\n", millisecondsSending);
    printf("Kernel Time:\t%f ms\n", millisecondsKernel);
    printf("Receiving Time:\t%f ms\n", millisecondsReceiving);
    printf("Overall Time:\t%f ms\n", milliseconds);
#endif
}
