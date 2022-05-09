/*!
	Project: libopengrn
	File: darray.h
	Simple dynamic array

	Copyright (C) 2021 Arves100
*/
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "dllapi.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct SDArray {
    size_t count;
    size_t elementSize;
    size_t reserved;
    uint8_t *data;
} TDArray;

extern OG_DLLAPI void DArray_Free(TDArray *a);

extern OG_DLLAPI bool DArray_Init(TDArray *a, size_t elementSize, size_t initialSize);

extern OG_DLLAPI bool DArray_Resize(TDArray *a, size_t newSize);

extern OG_DLLAPI bool DArray_Add(TDArray *a, void *element);

extern OG_DLLAPI void *DArray_Get(TDArray *a, size_t idx);
#ifdef __cplusplus
}
#endif
