/*!
	Project: libopengrn
	File: darray.c
	Simple dynamic array

	Copyright (C) 2021 Arves100
*/
#include "darray.h"
#include "debug.h"

#include <stdlib.h>

void OG_DLLAPI DArray_Free(TDArray* a)
{
	a->count = 0;
	a->elementSize = 0;
	a->reserved = 0;

	if (a->data)
	{
		free(a->data);
		a->data = NULL;
	}
}

bool OG_DLLAPI DArray_Init(TDArray* a, size_t elementSize, size_t initialSize)
{
	if (initialSize)
	{
		a->data = (uint8_t*)malloc(initialSize * elementSize);
		if (!a->data)
		{
			dbg_printf("darray malloc fail");
			return false;
		}
	}
	else
		a->data = NULL;

	a->count = 0;
	a->reserved = initialSize;
	a->elementSize = elementSize;
	return true;
}

OG_DLLAPI void* DArray_Get(TDArray* a, size_t idx)
{
	if (idx > a->count)
	{
		dbg_printf("darray get out of bounds %zu is bigger than %zu", idx, a->count);
		return NULL;
	}

	return a->data + (a->elementSize * idx);
}

bool OG_DLLAPI DArray_Resize(TDArray* a, size_t newSize)
{
	uint8_t* ptr = (uint8_t*)realloc(a->data, a->elementSize * newSize);

	if (!ptr)
	{
		dbg_printf("darray realloc of %zu fail", newSize);
		return false;
	}

	a->data = ptr;
	a->reserved = newSize;
	return true;
}

bool OG_DLLAPI DArray_Add(TDArray* a, void* element)
{
	if ((a->count + 1) > a->reserved)
	{
		if (!DArray_Resize(a, a->count + 1))
			return false;
	}

	memcpy_s(a->data + (a->count * a->elementSize), a->elementSize * a->reserved, element, a->elementSize);
	a->count++;
	return true;
}