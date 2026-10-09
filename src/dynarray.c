#include "../inc/dynarray.h"
#include "../inc/logger.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    size_t size;
    size_t capacity;
    size_t data_size;
} dynarray_t;

void* dynarray_init(size_t data_size, size_t array_size) {
    dynarray_t* array = (dynarray_t*)malloc(
        sizeof(dynarray_t) + data_size*array_size
    );
    array->size = 0;
    array->capacity = array_size;
    array->data_size = data_size;
    return array+1;
}

void dynarray_fini(void* dynarray) {
    if (dynarray) {
        dynarray_t* array = ((dynarray_t*)dynarray)-1;
        free(array);
    }
}
void dynarray_clear(void* dynarray) {
    dynarray_t* array = ((dynarray_t*)dynarray)-1;
    array->size = 0;
}
size_t dynarray_pop(void* dynarray) {
    dynarray_t* array = ((dynarray_t*)dynarray)-1;
    if (array->size) return --array->size;
    return 0;
}
size_t dynarray_push(void** dynarray) {
    dynarray_t* array = ((dynarray_t*)*dynarray)-1;
    if (array->size >= array->capacity) {
        array->capacity *= 2;
        array = realloc(
            array,
            sizeof(dynarray_t) +
            array->data_size*array->capacity
        );
        if (!array) {
            logger_err("Out of memory? Abort");
            exit(EXIT_FAILURE);
        }
        *dynarray = array+1;
    }
    return array->size++;
}
size_t dynarray_size(void* dynarray) {
    dynarray_t* array = ((dynarray_t*)dynarray)-1;
    return array->size;
}