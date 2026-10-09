#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stdlib.h>

void* dynarray_init(size_t data_size, size_t array_size);
void dynarray_fini(void* dynarray);

void dynarray_clear(void* dynarray);
size_t dynarray_pop(void* dynarray);
size_t dynarray_push(void** dynarray);
size_t dynarray_size(void* dynarray);

#endif