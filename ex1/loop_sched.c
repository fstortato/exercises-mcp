#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>


/***
 *
 * @param lower_bound   the number at which the first iteration is executed
 * @param upper_bound   the last iteration should be executed at (upper_bound-1)
 * @param results       a pointer to the results array for the whole loop execution
 ***/
void static_scheduling(const int lower_bound, const int upper_bound, int* results) {
    
    #pragma omp parallel shared(lower_bound, upper_bound, results) // Creates a parallel region
    {
        int i; // Creates a loop variable
        int n_threads = omp_get_num_threads(); // Get number of available threads to execute
        int c_thread = omp_get_thread_num(); // Get the number of the thread executing the current iteration
        int N = upper_bound - lower_bound; // Get the number of loop iterations
        
        /* 
        Start of loop: "Current" thread will compute from: (current_thread_number) * [(number_of_iterations) / (number_of_threads)] + lower_bound
        End of loop: "Current" thread will compute until, but not included: Start of lopp + [(number_of_iterations) / (number_of_threads)] +lower_bound = (current_thread_number + 1) * [(number_of_iterations) / (number_of_threads)] + lower_bound
        */
        int start_loop = c_thread * (N / n_threads) + lower_bound; // value for first index value for this thread
        int end_loop = (c_thread + 1) * (N / n_threads) + lower_bound; // value for last index value for this thread
        for(i=start_loop; i<end_loop; i++)
        {
            int start = i;
            int end = i + 1;
            compute(start, end, results);        // Function to be called to perform iterations
        }
    }
}

/***
 *
 * @param lower_bound   the number at which the first iteration is executed
 * @param upper_bound   the last iteration should be executed at (upper_bound-1)
 * @param results       a pointer to the results array for the whole loop execution
 ***/
void chunk_scheduling(const int lower_bound, const int upper_bound, int* results) {
   
    int CHUNK_SIZE = 1000;  // Define the chunk size as 1000
    int global_i = lower_bound; // Initialize global index as the lower_bound parameter
    #pragma omp parallel shared(global_i, CHUNK_SIZE, lower_bound, upper_bound, results) // Create a parallel region
    {
        /* 
        Start of loop: "Current" thread will compute the chunk from next available global index
        End of loop: "Current" thread will compute until [(Start of loop) + (chunk size)] or N
        */
        
        int start_loop = 0;
        int end_loop = 0;
        int local_i = 0;
        int i;

        do {
            #pragma omp critical // protect global counter and update local counter
            {
                local_i = global_i;
                global_i = global_i + CHUNK_SIZE;
            }
            start_loop = local_i; // Local loop starts at start i
            if((local_i + CHUNK_SIZE) < upper_bound) { // check if the chunk_size is not bigger then the remaining iterations
                end_loop = local_i + CHUNK_SIZE;
            }
            else {
                end_loop = upper_bound;
            }
            // Execute the function between given bounds
            for(i=start_loop; i<end_loop; i++) 
            {
                int start = i;
                int end = i + 1;
                compute(start, end, results); // Function to be called to perform iterations
            }
        } while (end_loop<upper_bound);

    }
}

/***
 *
 * @param lower_bound   the number at which the first iteration is executed
 * @param upper_bound   the last iteration should be executed at (upper_bound-1)
 * @param results       a pointer to the results array for the whole loop execution
 ***/
void guided_self_scheduling(const int lower_bound, const int upper_bound, int* results) {
    // Implementation is similar to dynamic, but now the chink size is varying
    int global_i = lower_bound;
    #pragma omp parallel shared(lower_bound, upper_bound, global_i, results) // Creates parallel region
    {
        /* 
        Start of loop: "Current" thread will compute the chunk from next available global index
        End of loop: "Current" thread will compute until [(Start of loop) + (chunk size)] or N
        */
        
        int start_loop = 0;
        int end_loop = 0;
        int local_i = 0;
        int local_block_size = 0;
        int i;

        do {
            #pragma omp critical // protect global counter and update local i
            {
                local_i = global_i;
                local_block_size = ceil(((double)(upper_bound - local_i))/omp_get_num_threads()); // calculation of size of the block of iterations to be mapped
                global_i = global_i + local_block_size;
            }
            start_loop = local_i;
            if((local_i + local_block_size) < upper_bound) { // Check if chunk size will not go beyond limits
                end_loop = local_i + local_block_size;
            }
            else {
                end_loop = upper_bound;
            }
            
            for(i=start_loop; i<end_loop; i++) // Perform operations
            {
                int start = i;
                int end = i + 1;
                compute(start, end, results); // Function to be called to perform iterations
            }
        } while (end_loop<upper_bound); // Keep scheduling until no more iterations need to be done

    }
   
}