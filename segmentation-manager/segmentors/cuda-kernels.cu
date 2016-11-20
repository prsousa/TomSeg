#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>

__global__
void zero_GPU( double *l_p_array_gpu ) {
    int i = blockIdx.x * blockDim.x + threadIdx.x; // <-- in case you use more blocks
    printf("  %i: Hello World!\n", i);
    l_p_array_gpu[i] = 0;
}

void zero(double *l_p_array, int a_numElements)
{
    double *l_p_array_gpu;

    int size = a_numElements * int(sizeof(double));

    cudaMalloc((void**) &l_p_array_gpu, size);

    cudaMemcpy(l_p_array_gpu, l_p_array, size, cudaMemcpyHostToDevice);

    // use one block with a_numElements threads
    zero_GPU<<<1, a_numElements>>>(l_p_array_gpu);

    cudaMemcpy(l_p_array, l_p_array_gpu, size, cudaMemcpyDeviceToHost);

    cudaFree(l_p_array_gpu);
}
