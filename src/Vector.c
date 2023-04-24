#include "include/Vector.h"

Vector vec_new(const size_t capacity, const size_t item_size, const AllocationMode allocation_mode) {
    Vector vec = {
        .capacity = capacity,
        .allocation_mode = allocation_mode,
        .len = 0,
        .item_size = item_size,
        .vector = malloc(item_size * capacity),
    };
    return vec;
}

bool vec_get(const Vector *self, const int index, void* return_value) {
    assert(self != NULL && return_value != NULL);
    if (self->len == 0) return false;
    size_t position = vec_get_valid_index(self, index);
    memmove(return_value, vec_get_addr_pos(self, position), self->item_size);
    return true;
}

bool vec_insert(Vector *self, const int index, const void *value) {
    assert(self != NULL && value != NULL);

    // Check index
    self->len++;
    size_t position = vec_get_valid_index(self, index);

    vec_reallocate_memory(self);
    if (self->vector == NULL) return false;
    
    // Move all values to the right
    if (self->capacity > position + 1){
        memmove(vec_get_addr_pos(self, position + 1), vec_get_addr_pos(self, position), (self->len - position) * self->item_size);
    }
    // Copy the value to the buffer
    memmove(vec_get_addr_pos(self, position), value, self->item_size);
    return true;
}

bool vec_replace_position(Vector *self, const int index, const void *value) {
    assert(self != NULL && value != NULL);

    // Check index
    size_t position = vec_get_valid_index(self, index);
    
    // Copy the value to the buffer
    memmove(vec_get_addr_pos(self, position), value, self->item_size);
    return true;
}

bool vec_push(Vector *self, const void *value) {
    return vec_insert(self, -1, value);
}

bool vec_pop(Vector *self, void *return_value) {
    return vec_remove(self, -1, return_value);
}

int vec_search(const Vector *self, const void *value, int (*comparison_fn)(const void *a,const void *b)) {
    for (size_t c = 0; c < self->len; ++c) {
        if (comparison_fn(vec_get_addr_pos(self, c), value) == 0) {
            return c;
        }
    }
    return -1;
}

int vec_search_binary(const Vector *self, const void *value, int (*comparison_fn)(const void *a,const void *b)){
    assert(self != NULL && comparison_fn != NULL && value != NULL);
    int lower_index = 0;
    int upper_index = self->len;
    int mid_point = 0;
    int comparison = 0;
    while (lower_index != upper_index) {
        mid_point = (upper_index - lower_index) / 2;
        comparison = comparison_fn(vec_get_addr_pos(self, mid_point) , value);
        if (comparison == 0) {
            return comparison;
        }
        else if (comparison < 0) {
            upper_index = mid_point;
        }
        else {
            lower_index = mid_point;
        }
    }
    return -1;
}

bool vec_remove(Vector *self, const int index, void *return_value) {
    assert(self != NULL);

    if (self->len == 0) return false;

    size_t position = vec_get_valid_index(self, index);

    if (return_value != NULL){
        // Copy the value to the destination
        memmove(return_value, vec_get_addr_pos(self, position), self->item_size);
    }
    self->len--;
    memmove(vec_get_addr_pos(self, position), vec_get_addr_pos(self, position+1), (self->len - position) * self->item_size);

    vec_reallocate_memory(self);
    return true;
}

bool vec_remove_search(Vector *self, const void *value, int (*comparison_fn)(const void *a,const void *b)) {
    int index = vec_search(self, value, comparison_fn);
    if (index < 0) return false;
    vec_remove(self, index, NULL);
    return true;
}

bool vec_is_empty(const Vector *self){
    assert(self != NULL);
    return self->len == 0;
}

void vec_free(Vector *self){
    assert(self != NULL);

    vec_clear(self);
}

void vec_reverse(Vector *self);

Vector vec_copy(const Vector *self) {
    assert(self != NULL);

    return vec_slice_copy(self, 0, self->len);
}

Vector vec_slice_copy(const Vector *self, const int index_first, const int index_last){
    assert(self != NULL);

    size_t index1 = vec_get_valid_index(self, index_first);
    size_t index2 = vec_get_valid_index(self, index_last);

    if (index2 <= index1) {
        return vec_new(0, self->item_size, self->allocation_mode);
    }

    size_t total_len = index2 - index1;

    Vector vec = vec_new(total_len, self->item_size, self->allocation_mode);
    
    memmove(vec.vector, vec_get_addr_pos(self, index1), total_len * self->item_size);
    
    return vec;
}

Vector vec_slice(Vector *self, int index_first, int index_last) {
    assert(self != NULL);
    size_t index1 = vec_get_valid_index(self, index_first);
    size_t index2 = vec_get_valid_index(self, index_last);

    if (index2 <= index1) {
        return vec_new(0, self->item_size, self->allocation_mode);
    }

    size_t total_len = index2 - index1;
    Vector vec = {
        .allocation_mode = self->allocation_mode,
        .capacity = total_len,
        .len = total_len,
        .item_size = self->item_size,
    };

    vec.vector = vec_get_addr_pos(self, index1);

    return vec;
}

void vec_change_allocation(Vector *self, AllocationMode new_mode){
    assert(self != NULL);
    self->allocation_mode = new_mode;
}

void vec_sort(Vector *self, int (*comparison_fn)(const void *a, const void *b)) {
    qsort(self->vector, self->len, self->item_size, comparison_fn);
}

void vec_reallocate_memory(Vector *self){
    assert(self != NULL);
    // First Allocation
    if (self->len == 0) {
        if (self->capacity == 0){
            self->capacity = 1;
        }
    }
    else {
        switch (self->allocation_mode)
        {
        case LOOSE:
            if (self->len >= self->capacity){
                self->capacity = self->len << 1;
            }
            else if ((self->capacity >> 1) > self->len){
                self->capacity >>= 1;
            }
            break;
        case STRICT:
            if (self->len >= self->capacity){
                self->capacity = self->len + 1;
            }
            else if (self->capacity > self->len){
                self->capacity = self->len;
            }
            break;
        }
    }

    self->vector = realloc(self->vector, self->capacity * self->item_size);
    if (self->vector == NULL){
        vec_clear(self);
    }
}

void vec_clear(Vector *self){
    if (self) {
        self->capacity = 0;
        self->len = 0;
        if (self->vector != NULL) {
            free(self->vector);
            self->vector = NULL;
        }
    }
}

Vector vec_filter(const Vector *self, bool (*func) (void *a)) {
    Vector vec = vec_new(self->len, self->item_size, self->allocation_mode);
    for (size_t c = 0; c < self->len; c++) {
        if (func(vec_get_addr_pos(self, c))) {
            vec_push(&vec, vec_get_addr_pos(self, c));
        }
    }
    return vec;
}

void vec_initialize(Vector *self, void *value){
    assert(self != NULL && value != NULL);

    for (size_t c = 0; c < self->len; c++) {
        vec_replace_position(self, c, value);
    }
}

inline void vec_swap(Vector *self, int index1, int index2){
    assert(self != NULL);
    size_t index_p1 = vec_get_valid_index(self, index1);
    size_t index_p2 = vec_get_valid_index(self, index2);
    char temp;
    
    for (size_t c = 0; c < self->item_size; c++) {
        temp = *(char *)(vec_get_addr_pos(self, index_p1) + c);
        *(char *)(vec_get_addr_pos(self, index_p1) + c) = *(char *)(vec_get_addr_pos(self, index_p2) + c);
        *(char *)(vec_get_addr_pos(self, index_p2) + c) = temp;
    }
}

size_t vec_get_valid_index(const Vector *self, const int index) {
    if (self->len != 0) {
        if (index < 0) { // Handle negative index
            int multiplier = ((index * -1) / (self->len + 1)) + 1;
            return self->len * multiplier + index;
        }
        else {
            return index % self->len;
        }
    }
    return 0;
}

void *vec_get_addr_pos(const Vector *self, const size_t index) {
    assert(self != NULL);
    return (void *) self->vector + (self->item_size * index);
}
