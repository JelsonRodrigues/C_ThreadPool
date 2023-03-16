#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include "src/include/ProducerConsumer.h"
#include "src/include/ThreadPool.h"

// This mutex is for the stdout, so only one thread writes at time
pthread_mutex_t mutex_stdout = PTHREAD_MUTEX_INITIALIZER;

void *f1(void *args) {
    long arg = (long) args;
    sleep(arg % 3);
    pthread_mutex_lock(&mutex_stdout);
    printf("Task %ld\n", arg);
    pthread_mutex_unlock(&mutex_stdout);
    return NULL;
}

int main() {
    srand(0);
    ThreadPool *thread_pool = new_thread_pool(3);

    Task task = {
        .args = NULL,
        .fn = f1,
        .ret = NULL,
        .has_finished = false,
    };
#define TASKS_TO_DO 17
    for (size_t c= 0; c < TASKS_TO_DO; ++c){
        task.args = (void *) ((long) rand());
        add_task(thread_pool, &task);
    }

     finish_executing_and_terminate(thread_pool);

    return EXIT_SUCCESS;
}