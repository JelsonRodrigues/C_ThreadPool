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
    ThreadPool *thread_pool = thread_pool_new(3);

    Task *task1 = task_new(f1, NULL, NULL);
    Task *task2 = task_new(f2, NULL, NULL);
    Task *task3 = task_new(f3, NULL, NULL);
#define TASKS_TO_DO 7
    for (size_t c= 0; c < TASKS_TO_DO; ++c){
        pthread_mutex_lock(&mutex_stdout);
        printf("Adding %ld iteration\n", c);
        pthread_mutex_unlock(&mutex_stdout);

        thread_pool_add_task(thread_pool, &task3);
        thread_pool_add_task(thread_pool, &task3);
        task1->args = (void *) ((long) rand());
        thread_pool_add_task(thread_pool, &task1);
        task2->args = (void *) ((long) rand());
        thread_pool_add_task(thread_pool, &task2);
    }

    task_await(task1);
    task_await(task2);
    task_await(task3);
    
    thread_pool_finish_executing_and_terminate(thread_pool);

    task_free(task1);
    task_free(task2);
    task_free(task3);

    return EXIT_SUCCESS;
}