#include "virtual_ptr.h"

#include "darray.h"

static TDArray array;

uint32_t encode_ptr(const void *ptr) {
    if(array.data == 0) {
        DArray_Init(&array, sizeof(void*), 100);
    }

    uint32_t virtual_ptr = array.count + 1;
    DArray_Add(&array, &ptr);

    return virtual_ptr;
}

void* decode_ptr(uint32_t ptr) {
    if(ptr == 0) {
        return 0;
    }

    if(array.data == 0) {
        DArray_Init(&array, sizeof(void*), 100);
    }

    if(ptr > array.count) {
        return 0;
    }

    return *(void**)DArray_Get(&array, ptr - 1);
}