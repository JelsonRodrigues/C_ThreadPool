#ifndef VECTOR_H
#define VECTOR_H

#ifndef __STDC_LIB_EXT1__
#define __STDC_LIB_EXT1__
#endif

#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/*
Allocation mode LOOSE: more CPU efficient, but less efficient in memory (more used memory)
Insert: will always allocate the double of current allocated space, if there is not engouth space
Remove: will only srink the size if the ration between used space and available space is
less than 50%

Allocation mode STRICT: less CPU efficient but less memory usage
Insert: will always allocate the size of only one item 
Remove: will always reduce the buffer space when item is removed
*/
typedef enum {LOOSE, STRICT} AllocationMode;

typedef struct {
    void *vector;
    size_t len;
    size_t capacity;
    const size_t item_size;
    AllocationMode allocation_mode;
} Vector;

Vector vec_new(const size_t capacity, const size_t item_size, const AllocationMode allocation_mode);

bool vec_get(const Vector *self, const int index, void* return_value);

bool vec_insert(Vector *self, const int index, const void *value);

bool vec_replace_position(Vector *self, const int index, const void *value);

bool vec_push(Vector *self, const void *value);

bool vec_pop(Vector *self, void *return_value);

int vec_search(const Vector *self, const void *value, int (*comparison_fn)(const void *a,const void *b));

int vec_search_binary(const Vector *self, const void *value, int (*comparison_fn)(const void *a,const void *b));

bool vec_remove(Vector *self, const int index, void *return_value);

bool vec_remove_search(Vector *self, const void *value, int (*comparison_fn)(const void *a,const void *b));

bool vec_is_empty(const Vector *self);

void vec_free(Vector *self);

void vec_reverse(Vector *self);

Vector vec_copy(const Vector *self);

Vector vec_slice_copy(const Vector *self, const int index_first, const int index_last);

Vector vec_slice(Vector *self, int index_first, int index_last);

void vec_change_allocation(Vector *self, AllocationMode new_mode);

void vec_sort(Vector *self, int (*comparison_fn)(const void *a, const void *b));

void vec_reallocate_memory(Vector *self);

void vec_clear(Vector *self);

Vector vec_filter(const Vector *self, bool (*func) (void *a));

void vec_initialize(Vector *self, void *value);

void vec_swap(Vector *self, int index1, int index2);

size_t vec_get_valid_index(const Vector *self, const int index);

void *vec_get_addr_pos(const Vector *self, const size_t index);

#endif