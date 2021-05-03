#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>

__global__ void find_min_device(float* in_vector, int N, float* grid_results) {

    /* Here I have tried to create this shared memory region, but it didn't work, as the values could not be set
    during runtime, it would need to be a const int or a #define value
    __shared__ float block_results[blockDim.x];
    To solve this, my solution was to create this shared space with the maximum number of threads possible. Here,
    I am considering 512 threads (I read it could be up to 1024 in more recent versions, but I was not sure about it).
    If the number of threads is smaller, this is no problem, as the program access only up to "blockDim.x", which is < 512
    */
    __shared__ float block_results[512];
    int tid = threadIdx.x + (blockIdx.x * blockDim.x); // Thread id inside the block added to the number of previous threads in the other previous blocks
    int i, size, thread_index = threadIdx.x;
    float part_min;

    /* Choice of cyclic iteration, then elements close to each other in memory can run in parallel by other threads */

    part_min = in_vector[tid]; /* Give part_min a start value (the first element) -> this adds an extra useless iteration.
    However I tought it might be better to have one more iteration than having another condition inside the for loop */
    for (i = tid; i < N; i += (blockDim.x * gridDim.x)) // Cyclic iterations as N > (blockDim.x*gridDim.x)
        if (in_vector[i] < part_min) part_min = in_vector[i]; // Check if the memory location at the new iteration has a smaller value
    block_results[thread_index] = part_min; // Store results for each thread in the array block_results

    __syncthreads(); // Wait all threads in the block

    size = (blockDim.x) / 2; /* Value of blockDim.x is different than length of block_results[], then the values after the
    limits of num_threads will be ignored */
    while (size != 0)
    {
        if (thread_index < size)
            if (block_results[thread_index + size] < block_results[thread_index]) block_results[thread_index] = block_results[thread_index + size];

        __syncthreads();
        size = size / 2;
    }

    /* Store results of this block in the grid results, to be computed later in the host */
    if (thread_index == 0)
        grid_results[blockIdx.x] = block_results[0];
}


float find_min(float* in_vector, int N, int block_size, int num_blocks)
{
    /* Creates pointers for in_vectortors in the host and in the device */
    float* in_vector_d;
    float* g_min, * g_min_d;

    /* Variable to store the computed minimum value */
    float final_min;

    /* Allocate memory for the in_vectortor that will receive the array with minimum value of each block */
    g_min = (float*)malloc(num_blocks * sizeof(float));

    /* Allocate memory in the GPU for the input in_vectortor and for the output in_vectortor with block minimums */
    cudaMalloc((void**)&in_vector_d, N * sizeof(float));
    cudaMalloc((void**)&g_min_d, num_blocks * sizeof(float));

    /* Copy initialized in_vectortor from the host to the device */
    cudaMemcpy(in_vector_d, in_vector, N * sizeof(float), cudaMemcpyHostToDevice);

    /* Call the function on the device */
    /* 1D arrangment */
    find_min_device << <num_blocks, block_size >> > (in_vector_d, N, g_min_d);

    /* Copy results of minimum of each block from the device to the host */
    cudaMemcpy(g_min, g_min_d, num_blocks * sizeof(float), cudaMemcpyDeviceToHost);

    /* Calculate the minimum value */
    final_min = g_min[0];
    for (int i = 1; i < num_blocks; i++)
        if (g_min[i] < final_min) final_min = g_min[i];

    //printf("Minimum value: %f", final_min);

    /* Free allocated memory space in the device and in the host */
    cudaFree(in_vector_d);
    cudaFree(g_min_d);
    free(g_min);

    /* Return to main function */
    return final_min;

}


int main(void) {

    /* Test parameters */
    int N = 1024 * 256;
    int block_size = 256;
    int num_blocks = 128;

    float* in_vector;
    in_vector = (float*)malloc(N * sizeof(float));

    /* Initializes random number generator */
    srand(time(NULL));

    /* Initializes random in_vectortor */
    for (int i = 0; i < N; i++) {
        in_vector[i] = (rand() % N) - 7;
        //printf("%.f\t", in_vector[i]);
    }
    //printf("\n");
    printf("Min: %.0f", find_min(in_vector, N, block_size, num_blocks));
    return 0;
}