/*!
	Project: libopengrn
	File: debug.c
	Debug utilities

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "elements.h"
#include "debug.h"
#include <stdarg.h>

#ifdef _DEBUG

void dbg_printf_real(const char* file, size_t line, const char* fmt, ...)
{
	va_list vl;

	printf("[%s:%zu] >> ", file, line);

	va_start(vl, fmt);
	vprintf(fmt, vl);
	va_end(vl);
}

void dbg_printf_no_line(const char* fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	vprintf(fmt, vl);
	va_end(vl);
}

void dbg_printelement(struct SElementInfo* elem)
{
	int32_t as = elem->info.arraySize;

	if (!as)
		as = 1;

	switch (elem->info.type) // types that do require manual handling
	{
	case TYPEID_REFERENCE: // 2
	case TYPEID_EMPTYREFERENCE: // 22
		printf(" %p", elem->data);
		return;
	case TYPEID_STRING: // 8
		printf(" %s", (char*)elem->data);
		return;
	case TYPEID_VARIANTREFERENCE: // 5
	{
		TVariantReference* ref = (TVariantReference*)elem->data;
		printf(" offset: %I64u data: %p", ref->offset, ref->data);
		break;
	}
	case TYPEID_REFERENCETOVARIANTARRAY: // 7
	{
		TReferenceToVariantArrayData* ref = (TReferenceToVariantArrayData*)elem->data;
		printf(" size: %u offset: %I64u data: %p", ref->size, ref->offset, ref->data);
		return;
		break;
	}
	case TYPEID_REFERENCETOARRAY: // 3
	{
		TReferenceToArrayData* ref = (TReferenceToArrayData*)elem->data;
		printf(" size: %u data: %p", ref->size, ref->data);
		return;
	}
	case TYPEID_ARRAYOFREFERENCES: // 4
	{
		TArrayReferenceData* ref = (TArrayReferenceData*)elem->data;
		printf(" size: %u", ref->size);

		for (uint32_t i = 0; i < ref->size; i++)
		{
			printf(" ptrpos %d: %p", i, ref->ptr[i]);
		}
		return;
	}
	case TYPEID_INLINE:
	case TYPEID_REMOVED:
		return;
	default:
		break;
	}

	for (int32_t i = 0; i < as; i++)
	{
		printf(" ");

		switch (elem->info.type)
		{
		case TYPEID_REAL32:
			printf("%f", ((float*)elem->data)[i]);
			break;
		case TYPEID_INT8:
		case TYPEID_BINORMALINT8:
			printf("%d", ((int8_t*)elem->data)[i]);
			break;
		case TYPEID_INT16:
		case TYPEID_BINORMALINT16:
			printf("%d", ((int16_t*)elem->data)[i]);
			break;
		case TYPEID_UINT16:
		case TYPEID_NORMALUINT16:
			printf("%u", ((uint16_t*)elem->data)[i]);
			break;
		case TYPEID_UINT8:
		case TYPEID_NORMALUINT8:
			printf("%u", ((uint8_t*)elem->data)[i]);
			break;
		case TYPEID_REAL16:
			printf("%f", (float)((int16_t*)elem->data)[i]);
			break;
		case TYPEID_INT32:
			printf("%d", ((int32_t*)elem->data)[i]);
			break;
		case TYPEID_UINT32:
			printf("%u", ((uint32_t*)elem->data)[i]);
			break;
		case TYPEID_TRANSFORM:
		{
			TTransformation tr = ((TTransformation*)elem->data)[i];
			printf("flags %u x %f y %f z %f rotationx %f rotationy %f rotationz %f rotationw %f scaleshear %f %f %f %f %f %f %f %f %f", tr.flags, tr.translation[0], tr.translation[1], tr.translation[2], tr.rotation[0], tr.rotation[1], tr.rotation[2], tr.translation[3],
				tr.scaleShear[0][0], tr.scaleShear[0][1], tr.scaleShear[0][2], tr.scaleShear[1][0], tr.scaleShear[1][1], tr.scaleShear[1][2], tr.scaleShear[2][0], tr.scaleShear[2][1], tr.scaleShear[2][2]);
			break;
		}
		}
	}
}

#endif // _DEBUG