#include "include/ThreadPool.h"

ThreadPool *new_thread_pool(size_t number_of_threads){
    ThreadPool new = {
        .threads = vec_new(number_of_threads, sizeof(pthread_t), STRICT),
        .communication = create_producer_consumer_buffer(MAX_WAITING_TASKS, sizeof(Task)),
    };

    ThreadPool *new_pool = (ThreadPool *) malloc(sizeof(ThreadPool));
    if (new_pool == NULL) return NULL;
    
    memmove(new_pool, &new, sizeof(ThreadPool));
    
    add_producer_to_buffer(&new_pool->communication);

    for (size_t c = 0; c < number_of_threads; c++) {
        add_consumer_to_buffer(&new_pool->communication);
        pthread_t tid;
        pthread_create(&tid, NULL, runner, &new_pool->communication);
        vec_push(&new_pool->threads, &tid);
    }

    return new_pool;
}

void *runner(void *args) {
    ProducerConsumerBuffer *buff =(ProducerConsumerBuffer *) args;

    bool received = false;
    do {
        Task task;
        received = get_item_from_buffer(buff, &task);

        if (received){
            task.has_finished = false;
            task.ret = task.fn(task.args);
            task.has_finished = true;
        }
    } while (received);

    remove_consumer_from_buffer(buff);

    return NULL;
}

bool try_add_task(ThreadPool *self, Task *new_task) {
    assert(self != NULL && new_task != NULL);

    return try_insert_to_buffer(&self->communication, new_task);
}
bool add_task(ThreadPool *self, Task *new_task){
    assert(self != NULL && new_task != NULL);
    return insert_to_buffer(&self->communication, new_task);
}
void release_pool(ThreadPool *self){
    assert(self != NULL);
    for (size_t c = 0; c < self->threads.len; c++) {
        pthread_t tid;
        vec_get(&self->threads, c, (void *) &tid);
        pthread_cancel(tid);
    }
    vec_free(&self->threads);

    free_ProducerConsumerBuffer(&self->communication);
    
    free(self);
}

void finish_executing_and_terminate(ThreadPool *self){
    assert(self != NULL);
    int number_of_producers = 0;
    sem_getvalue(&self->communication.total_producers, &number_of_producers);
    
    for (int c = 0; c < number_of_producers; ++c) {
        remove_producer_from_buffer(&self->communication);
    }

    size_t number_of_items = self->threads.len;
    for (size_t c = 0; c < number_of_items; ++c){
        pthread_t tid;
        vec_remove(&self->threads, c, &tid);
        pthread_join(tid, NULL);
    }

    release_pool(self);
}