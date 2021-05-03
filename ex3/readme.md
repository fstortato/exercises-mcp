## Task
Implement an NVIDIA CUDA program which determines the smallest element of a vector "in_vector" with N Float elements. 
The number of blocks and the block size are given.
Assume that N > block_size * num_blocks. 

Implement using these steps:
* Implementation of the host function: float find_min(float* in_vector, int N, int block_size, int num_blocks), containing:
** CUDA memory operations
** Call to the device function
** The final computation of global minimum
** Minimum as return value

* Implementation of the device function: void find_min_device(float* in_vector, int N, float* grid_results), assuming:
** Each thread determines the local minimum of its assigned vector elements
** Threads of a block determine one local minimum for this block in parallel