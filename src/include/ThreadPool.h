#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "ProducerConsumer.h"

typedef struct {
    Vector threads;
    ProducerConsumer communication;
} ThreadPool;

typedef struct {
    void * (*fn) (void *);
    void *args;
    void *ret;
    bool has_finished;
} Task;

#define MAX_WAITING_TASKS 256

ThreadPool *new_thread_pool(size_t number_of_threads);
bool try_add_task(ThreadPool *self, Task *new_task);
bool add_task(ThreadPool *self, Task *new_task);
void release_pool(ThreadPool *self);
void *runner(void *args);
void finish_executing_and_terminate(ThreadPool *self);
#endif
