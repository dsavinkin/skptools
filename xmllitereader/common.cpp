#include "common.h"

//#include <ole2.h>
//#include <xmllite.h>
#include <stdio.h>
#include <shlwapi.h>
#include <assert.h>

void array_init(ARRAY_T *a, size_t element_size)
{
    assert(element_size > 0);
    a->array = NULL;
    a->size = 0;
    a->used = 0;
    //Allign element_size
    a->element_size = ((element_size + 7) / 8) * 8;
}

void array_insert(ARRAY_T *a, void *element)
{
    if (a->used == a->size)
    {
        if (a->size == 0)
        {
            a->size = 2;
        }
        else
        {
            a->size *= 2;
        }
        a->array = (unsigned char *)realloc(a->array, a->size * a->element_size);
    }
    memcpy(&a->array[a->used++ * a->element_size], element, a->element_size);
}

void *array_get_element(ARRAY_T *a, size_t pos)
{
    return (void*)&a->array[a->used * a->element_size];
}

size_t array_get_count(ARRAY_T *a)
{
    return a->used;
}

void array_free(ARRAY_T *a)
{
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}
