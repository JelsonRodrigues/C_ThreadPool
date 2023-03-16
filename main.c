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
    printf("Task f1 %ld\n", arg);
    pthread_mutex_unlock(&mutex_stdout);
    return NULL;
}

#define MAX_FOR_COUNT 10000000

void *f2(void * args) {
    long arg = (long) args % MAX_FOR_COUNT;
    for (long c = 0;c < arg; ++c);
    pthread_mutex_lock(&mutex_stdout);
    printf("Task f2 %ld\n", arg);
    pthread_mutex_unlock(&mutex_stdout);
    return NULL;
}

void *f3(void * args) {
    pthread_mutex_lock(&mutex_stdout);
    printf("Task f3\n");
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
    Task task2 = {
        .args = NULL,
        .fn = f2,
        .ret = NULL,
        .has_finished = false,
    };
    Task task3 = {
        .args = NULL,
        .fn = f3,
        .ret = NULL,
        .has_finished = false,
    };
#define TASKS_TO_DO 7
    for (size_t c= 0; c < TASKS_TO_DO; ++c){
        add_task(thread_pool, &task3);
        add_task(thread_pool, &task3);
        task.args = (void *) ((long) rand());
        add_task(thread_pool, &task);
        task2.args = (void *) ((long) rand());
        add_task(thread_pool, &task2);
    }

    finish_executing_and_terminate(thread_pool);

    return EXIT_SUCCESS;
}