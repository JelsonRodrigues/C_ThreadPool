#include "include/ProducerConsumer.h"

Vector buffers = {
    .allocation_mode = LOOSE,
    .item_size = sizeof(ProducerConsumer),
};

// Create a new buffer and return the id to the caller
size_t create_new_buffer(size_t with_capacity, size_t sizeof_items) {
    ProducerConsumer last;
    size_t id = 0;
    if (vec_get(&buffers, -1, (void *) &last)) {
        id = last.id + 1;
    }
    
    ProducerConsumer buf = {
        .buffer = create_producer_consumer_buffer(with_capacity, sizeof_items),
        .id = id,
    };

    vec_push(&buffers, (void *) &buf);

    return id;
}

int comparison_function(const void *a, const void *b) {
    return ((ProducerConsumer *) a)->id - ((ProducerConsumer *) b)->id;
}

ProducerConsumer get_producer_consumer(size_t id_buffer) {
    ProducerConsumer temp = {.id = id_buffer};
    int index = vec_search(&buffers, (void *) &temp, comparison_function);
    if (index != -1) {
        vec_get(&buffers, index, (void *)&temp);
    } 
    
    return temp;
}

bool add_producer(size_t id_buffer) {
    ProducerConsumer temp = {.id = id_buffer};
    int index = vec_search(&buffers, (void *) &temp, comparison_function);
    if (index != -1) {
        vec_get(&buffers, index, (void *)&temp);
        add_producer_to_buffer(&temp.buffer);
        vec_replace_position(&buffers, index, (void*)&temp);
        return true;
    } 

    return false;
}

bool add_consumer(size_t id_buffer) {
    ProducerConsumer temp = {.id = id_buffer};
    int index = vec_search(&buffers, (void *) &temp, comparison_function);
    if (index != -1) {
        vec_get(&buffers, index, (void *)&temp);
        add_consumer_to_buffer(&temp.buffer);
        vec_replace_position(&buffers, index, (void*)&temp);
        return true;
    } 

    return false;
}

bool add_consumer_to_buffer(ProducerConsumerBuffer *buf){
    sem_post(&buf->total_consumers);
    return true;
}

bool add_producer_to_buffer(ProducerConsumerBuffer *buf){
    sem_post(&buf->total_producers);
    return true;
}

bool remove_consumer_from_buffer(ProducerConsumerBuffer *buf){
    sem_trywait(&buf->total_consumers);
    check_closing_channel(buf);

    return true;
}

bool remove_producer_from_buffer(ProducerConsumerBuffer *buf){
    sem_trywait(&buf->total_producers);
    check_closing_channel(buf);

    return true;
}

void check_closing_channel(ProducerConsumerBuffer *buf) {
    // Check the state of producers
    bool has_producers = has_producers_buffer(*buf);
    bool has_consumers = has_consumers_buffer(*buf);

    // Working normal
    if (has_producers && has_consumers) return;

    // Signal producers to stop waiting
    if (has_producers && !has_consumers) {
        while (sem_trywait(&buf->total_producers) == 0){
            sem_post(&buf->empty_places);        
        }
    }

    // Signal consumers to stop waiting
    if (!has_producers && has_consumers) {
        int total_consumers = 0;
        sem_getvalue(&buf->total_consumers, &total_consumers);

        for (int c = 0; c < total_consumers; c++){
            sem_post(&buf->filled_places);
        }
    }
}

bool has_producers_buffer(ProducerConsumerBuffer buf){
    bool locked = sem_trywait(&buf.total_producers) == 0; 
    if (locked) {
        sem_post(&buf.total_producers);
    }
    return locked;
}
bool has_consumers_buffer(ProducerConsumerBuffer buf){
    bool locked = sem_trywait(&buf.total_consumers) == 0; 
    if (locked) {
        sem_post(&buf.total_consumers);
    }
    return locked;
}

bool has_producers(ProducerConsumer buf){
    return has_producers_buffer(buf.buffer);
}
bool has_consumers(ProducerConsumer buf){
    return has_consumers_buffer(buf.buffer);
}

bool has_items_to_consume(ProducerConsumerBuffer *buf){
    int value = 0;
    sem_getvalue(&buf->empty_places, &value);

    if (value < (int) buf->capacity) {
        return true;
    }

    return false;
}

ProducerConsumerBuffer create_producer_consumer_buffer(size_t with_capacity, size_t sizeof_items) {
    ProducerConsumerBuffer buffer = {
        .sizeof_item = sizeof_items,
        .capacity = with_capacity,
        .next_insert_position = 0,
        .next_read_position = 0,
        .buffer = (void *) calloc(sizeof_items, with_capacity),
        .buffer_read_lock = PTHREAD_MUTEX_INITIALIZER,
        .buffer_write_lock = PTHREAD_MUTEX_INITIALIZER,
        };
    sem_init(&buffer.empty_places, 0, with_capacity);
    sem_init(&buffer.filled_places, 0, 0);
    sem_init(&buffer.total_consumers, 0, 0);
    sem_init(&buffer.total_producers, 0, 0);

    return buffer;
}

// Make a copy of value and store in buffer
bool insert_to_buffer(ProducerConsumerBuffer *buffer, void *value) {
    // Check the pointers
    if (buffer == NULL || buffer->buffer == NULL) {
        return false;
    }

    if (!has_consumers_buffer(*buffer)) return false;
    // Wait until the buffer have an empty place
    sem_wait(&buffer->empty_places);
    if (has_consumers_buffer(*buffer)){
        // Get the write lock, so only one producer writes at time
        pthread_mutex_lock(&buffer->buffer_write_lock);

        // Insert value and update index trackers
        memmove(buffer->buffer + buffer->sizeof_item * buffer->next_insert_position,value, buffer->sizeof_item);
        buffer->next_insert_position = (buffer->next_insert_position + 1) % buffer->capacity;
        
        // Release the write lock, so others can write
        pthread_mutex_unlock(&buffer->buffer_write_lock);
        // Increase the number of filled places, so consumers can read
        sem_post(&buffer->filled_places);

        return true;
    }
    return false;
}

// Try to insert an item into the buffer
// if there isnt available places, return false and dont insert or block the caller
bool try_insert_to_buffer(ProducerConsumerBuffer *buffer, void *value) {
    // Check the pointers
    if (buffer == NULL || buffer->buffer == NULL) {
        return false;
    }
    
    // Try to lock on the empty places, if there is no empty places, return false
    if (sem_trywait(&buffer->empty_places) == 0){
        // Get the write lock, so only one producer writes at time
        pthread_mutex_lock(&buffer->buffer_write_lock);

        // Insert value and update index trackers
        memmove(buffer->buffer + buffer->sizeof_item * buffer->next_insert_position,value, buffer->sizeof_item);
        buffer->next_insert_position = (buffer->next_insert_position + 1) % buffer->capacity;
        
        // Release the write lock, so others can write
        pthread_mutex_unlock(&buffer->buffer_write_lock);
        // Increase the number of filled places, so consumers can read
        sem_post(&buffer->filled_places);

        return true;
    }

    return false;
}

// Copy the item from buffer and stores in the area pointed to by return_value
// The area must be equal, or greater than buffer->sizeof_item
bool get_item_from_buffer(ProducerConsumerBuffer *buffer, void *return_value) {
    // Check the pointers
    if (buffer == NULL || buffer->buffer == NULL || return_value == NULL) {
        return false;
    }

    if (!has_producers_buffer(*buffer) && !has_items_to_consume(buffer)) return false;

    // Wait until there is some information in the buffer
    sem_wait(&buffer->filled_places);
    if (has_producers_buffer(*buffer) || has_items_to_consume(buffer)){
        // Get the read lock
        pthread_mutex_lock(&buffer->buffer_read_lock);

        // Copy the value to the return place and update the index for next read
        memmove(return_value, buffer->buffer + buffer->sizeof_item * buffer->next_read_position, buffer->sizeof_item);
        buffer->next_read_position = (buffer->next_read_position + 1) % buffer->capacity;

        // Release  the read lock, so other consumers can read from the buffer
        pthread_mutex_unlock(&buffer->buffer_read_lock);
        // Increase the total ammount of empty places, so producers can produce 
        sem_post(&buffer->empty_places);

        return true;
    }
    return false;
}

// Try to get an item from the buffer, if there is nothing in the buffer, returns false.
// The difference between this and the get_item_from_buffer is that on the other function the caller will
// be blocked until there is an item to get, what could led to starvation, this function
// check if there is something to get and if there isnt, simply return false
bool try_get_item_from_buffer(ProducerConsumerBuffer *buffer, void *return_value) {
    // Check the pointers
    if (buffer == NULL || buffer->buffer == NULL || return_value == NULL) {
        return false;
    }

    // Try to lock on the filled places, if there is no filled places return false
    if (sem_trywait(&buffer->filled_places) == 0) {
        // Get the read lock
        pthread_mutex_lock(&buffer->buffer_read_lock);

        // Copy the value to the return place and update the index for next read
        memmove(return_value, buffer->buffer + buffer->sizeof_item * buffer->next_read_position, buffer->sizeof_item);
        buffer->next_read_position = (buffer->next_read_position + 1) % buffer->capacity;

        // Release  the read lock, so other consumers can read from the buffer
        pthread_mutex_unlock(&buffer->buffer_read_lock);
        // Increase the total ammount of empty places, so producers can produce 
        sem_post(&buffer->empty_places);

        return true;
    }
    
    return false;
}

void free_ProducerConsumer(ProducerConsumer *object) {
    if (object) {
        free_ProducerConsumerBuffer(&object->buffer);
    }
}

void free_ProducerConsumerBuffer(ProducerConsumerBuffer *object) {
    if (object) {
        pthread_mutex_destroy(&object->buffer_read_lock);
        pthread_mutex_destroy(&object->buffer_write_lock);
        sem_destroy(&object->empty_places);
        sem_destroy(&object->filled_places);
        sem_destroy(&object->total_consumers);
        sem_destroy(&object->total_producers);
        free(object->buffer);
    }
}

void free_all_buffers() {
    for (size_t c = 0; c < buffers.len; c++) {
        free_ProducerConsumer(vec_get_addr_pos(&buffers, c));
    }
    vec_free(&buffers);
}
