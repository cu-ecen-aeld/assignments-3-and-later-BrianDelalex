#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    struct thread_data *data = (struct thread_data *) thread_param;
    if (!data)
        return NULL;
    
    usleep(data->wait_to_obtain_ms * 1000);
    int rc = pthread_mutex_lock(data->mutex);
    if (rc != 0) {
        printf("pthread_mutex_lock failed with %d\n", rc);
        data->thread_complete_success = false;
        return thread_param;
    }
    usleep(data->wait_to_release_ms * 1000);
    rc = pthread_mutex_unlock(data->mutex);
    if (rc != 0) {
        printf("pthread_mutex_unlock failed with %d\n", rc);
        data->thread_complete_success = false;
        return thread_param;
    }
    data->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    struct thread_data *data = malloc(sizeof(struct thread_data));
    
    if (!data) {
        printf("OUT OF MEMORY !\n");
        return false;
    }

    data->mutex = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    data->thread_complete_success = true;

    int rc = pthread_create(thread, NULL, threadfunc, (void *)data);
    if (rc != 0) {
        printf("pthread_create failed with %d\n", rc);
        free(data);
        return false;
    }

    return true;
}

