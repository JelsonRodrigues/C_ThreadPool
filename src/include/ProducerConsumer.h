#ifndef PRODUCERCONSUMER_H
#define PRODUCERCONSUMER_H

#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <string.h>
#include "Vector.h"

// This buffer works as a queue (fifo)
typedef struct {
    const size_t sizeof_item;           // Size of each item in the buffer
    void *buffer;                       // The buffer
    size_t next_insert_position;        // Next position for inserting an item
    size_t next_read_position;          // Next index position for reading a value
    const size_t capacity;              // The total allocated capacity
    sem_t empty_places;                 // Semaphore with the empty places in the buffer
    sem_t filled_places;                // Semaphore with the full places in the buffer
    pthread_mutex_t buffer_write_lock;  // Buffer write lock
    pthread_mutex_t buffer_read_lock;   // Buffer read lock
    sem_t total_consumers;              // Number of consumers
    sem_t total_producers;              // Number of producers
} ProducerConsumerBuffer;


typedef struct {
    ProducerConsumerBuffer buffer;
    size_t id;
} ProducerConsumer;

size_t create_new_buffer(size_t with_capacity, size_t sizeof_items);
ProducerConsumer get_producer_consumer(size_t id_buffer);
bool add_producer(size_t id_buffer);
bool add_consumer(size_t id_buffer);
bool add_producer_to_buffer(ProducerConsumerBuffer *buf);
bool add_consumer_to_buffer(ProducerConsumerBuffer *buf);
bool remove_producer_from_buffer(ProducerConsumerBuffer *buf);
bool remove_consumer_from_buffer(ProducerConsumerBuffer *buf);
void check_closing_channel(ProducerConsumerBuffer *buf);
bool has_producers(ProducerConsumer buf);
bool has_consumers(ProducerConsumer buf);
bool has_producers_buffer(ProducerConsumerBuffer buf);
bool has_consumers_buffer(ProducerConsumerBuffer buf);
bool has_items_to_consume(ProducerConsumerBuffer *buf);
ProducerConsumerBuffer create_producer_consumer_buffer(size_t with_capacity, size_t sizeof_items);
bool insert_to_buffer(ProducerConsumerBuffer *buffer, void *value);
bool try_insert_to_buffer(ProducerConsumerBuffer *buffer, void *value);
bool get_item_from_buffer(ProducerConsumerBuffer *buffer, void *return_value);
bool try_get_item_from_buffer(ProducerConsumerBuffer *buffer, void *return_value);
void free_ProducerConsumer(ProducerConsumer *object);
void free_ProducerConsumerBuffer(ProducerConsumerBuffer *object);
void free_all_buffers();
#endif