#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "ProducerConsumer.h"
#include "pthread.h"
#include "semaphore.h"

typedef struct {
    Vector threads;
    ProducerConsumerBuffer communication;
} ThreadPool;

typedef struct {
    void * (*fn) (void *);
    void *args;
    void *ret;
    bool has_finished;
    sem_t waiting_count;
    sem_t wait_queue;
} Task;

#define MAX_WAITING_TASKS 256

ThreadPool *thread_pool_new(size_t number_of_threads);
bool thread_pool_try_add_task(ThreadPool *self, Task **new_task);
bool thread_pool_add_task(ThreadPool *self, Task **new_task);
Task *task_new(void * (*fn) (void *), void *args, void *ret);
void task_await(Task *self);
void task_free(Task *self);
void thread_pool_release(ThreadPool *self);
void *runner(void *args);
void thread_pool_finish_executing_and_terminate(ThreadPool *self);
#endif
