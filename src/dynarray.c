#include "../inc/dynarray.h"
#include <string.h>
#include <stdio.h>

typedef struct _dynarray_t {
    size_t size;
    size_t capacity;
    size_t data_size;
} dynarray_t, *dynarray_p;

#define dynarray_data(dynarray) (((dynarray_p)dynarray)-1)

void* dynarray_init(size_t data_size, size_t array_size) {
    dynarray_p array = (dynarray_p)malloc(
        sizeof(dynarray_t) + data_size*array_size
    );
    array->size = 0;
    array->capacity = array_size;
    array->data_size = data_size;
    return array + 1;
}

void dynarray_fini(void* dynarray) {
    if (dynarray) free(dynarray_data(dynarray));
}
void dynarray_clear(void* dynarray) {
    dynarray_data(dynarray)->size = 0;
}
size_t dynarray_pop(void* dynarray) {
    dynarray_p array = dynarray_data(dynarray);
    return array->size ? --array->size : 0;
}
size_t dynarray_push(void** dynarray) {
    dynarray_p array = dynarray_data(*dynarray);
    if (array->size >= array->capacity) {
        array->capacity *= 2;
        array = (dynarray_p)realloc(
            array,
            sizeof(dynarray_t) +
            array->data_size*array->capacity
        );
        if (!array) {
            printf("Out of memory? Abort\n");
            exit(EXIT_FAILURE);
        }
        *dynarray = array + 1;
    }
    return array->size++;
}
size_t dynarray_size(void* dynarray) {
    return dynarray_data(dynarray)->size;
}