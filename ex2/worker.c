#include <stdio.h>
#include <pthread.h>
#include <errno.h>

#define NUM_WORK 25

/* Create a mutex for each of the gloves */
pthread_mutex_t l_gloves[NUM_WORK];
/* Create a mutex for the wrench */
pthread_mutex_t l_wrench;

int exit_flag = 0; // Used to stop the program after event

void* worker(void* id)
{
    /* Read id of the worker, cast to integer pointer and de-reference */
    int w_id = *((int*)id);
    /* Left glove has the same id as the worker id */
    int left = w_id;
    /* Right glove has woker id + 1 (right/anti-clockwise)*/
    int right = (w_id + 1) % NUM_WORK; // right glove is either id+1 or 0, in case of the last worker in the circle

    while (exit_flag == 0)
    {
        int first = right; // As indicated, the worker must take first the right glove
        int second = left; // Then, the left glove
        int status; // Variable to store the status of the lock try

        printf("Worker %d connected the parts.\n", w_id);
        fflush(stdout);

        /* Lock the first glove */
        pthread_mutex_lock(&l_gloves[first]);
        //printf("Worker %d picked up the first glove: %d.\n", w_id, first);
        //fflush(stdout);

        /* Try to lock the second glove and checks the message */
        status = pthread_mutex_trylock(&l_gloves[second]);
        if (status == EBUSY) // case the glove is already taken
        {
            pthread_mutex_unlock(&l_gloves[first]); // If glove[second="left"] is already taken, leave first glove to avoid deadlock
            //printf("Worker %d put back the first glove: %d.\n", w_id, first);
            //fflush(stdout);
        }
        else // case the glove was free. In this case, it is now taken by worker <id>
        {
            //printf("Worker %d picked up the second glove: %d.\n", w_id, second);
            //fflush(stdout);

            /* Lock the wrench */
            /* Assuming the lock in the wrench cannot cause a deadlock, as the wrench is seem with the same "wrench-id" by
            all the workers, i.e. there is no problem with one worker seing it the id "left" and other as "right", as it
            could happen in the case of the gloves */
            pthread_mutex_lock(&l_wrench);
            //printf("Worker %d picked up the wrench.\n", w_id);
            //fflush(stdout);

            printf("Worker %d tightened the bolts on the casing.\n", w_id);
            fflush(stdout);

            /* Unlock wrench */
            pthread_mutex_unlock(&l_wrench);
            //printf("Worker %d put back the wrench.\n", w_id);
            //fflush(stdout);

            /* Unlock second glove */
            pthread_mutex_unlock(&l_gloves[second]);
            //printf("Worker %d put back the second glove: %d.\n", w_id, second);
            //fflush(stdout);

            /* Unlock first glove */
            pthread_mutex_unlock(&l_gloves[first]);
            //printf("Worker %d put back the first glove: %d.\n", w_id, first);
            //fflush(stdout);

        }

    }

    return NULL;
}

int main(void) {

    int i = 0;

    /* Initialize locks for gloves */
    for (i = 0; i < NUM_WORK; i++)
        pthread_mutex_init(&l_gloves[i], NULL);

    /* Initialize lock for wrench */
    pthread_mutex_init(&l_wrench, NULL);

    /* Create a thread and an id for each worker */
    pthread_t threads[NUM_WORK];
    int ids[NUM_WORK];
    for (i = 0; i < NUM_WORK; i++)
    {
        ids[i] = i;
        pthread_create(&threads[i], NULL, worker, &ids[i]); // Create thread to execute the function worker
    }

    // When key press, while loop is interrupted
    getchar();
    exit_flag = 1;

    // Join threads
    for (i = 0; i < NUM_WORK; i++)
        pthread_join(threads[i], NULL);

    /* Destroy mutexes for gloves */
    for (i = 0; i < NUM_WORK; i++)
        pthread_mutex_destroy(&l_gloves[i]);

    /* Destroy mutex for the wrench */
    pthread_mutex_destroy(&l_wrench);

    return 0;
}
