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
    sleep(1);
    pthread_mutex_lock(&mutex_stdout);
    printf("Task 1\n");
    pthread_mutex_unlock(&mutex_stdout);
    return NULL;
}

int main() {
    ThreadPool *thread_pool = new_thread_pool(3);

    Task task = {
        .args = NULL,
        .fn = f1,
        .ret = NULL,
        .has_finished = false,
    };
    add_task(thread_pool, &task);
    add_task(thread_pool, &task);
    add_task(thread_pool, &task);
    add_task(thread_pool, &task);
    add_task(thread_pool, &task);

    remove_producer_from_buffer(&thread_pool->communication.buffer);
    sleep(6);
    release_pool(thread_pool);
    // finish_executing_and_terminate(thread_pool);

    return EXIT_SUCCESS;
}