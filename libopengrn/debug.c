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
#include "typeinfo.h"
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

void dbg_printelement(TElementGeneric* elem)
{
	switch (elem->rawInfo.type) // types that do require manual handling
	{
	case TYPEID_REFERENCE: // 2
	case TYPEID_EMPTYREFERENCE: // 22
	case TYPEID_VARIANTREFERENCE: // 5
		printf(" offset: %I64u data: %p", ((TElementReference*)elem)->offset, ((TElementReference*)elem)->reference);
		return;
	case TYPEID_STRING: // 8
		printf(" %s", ((TElementString*)elem)->value);
		return;
	case TYPEID_REFERENCETOVARIANTARRAY: // 7
	case TYPEID_REFERENCETOARRAY: // 3
	{
		TElementArray* ref = (TElementArray*)elem;
		printf(" size: %u offset: %I64u data: %p", ref->base.size, ref->offset, ref->data);
		return;
		break;
	}
	case TYPEID_ARRAYOFREFERENCES: // 4
	{
		TElementArray* ref = (TElementArray*)elem;
		printf(" size: %u offset: %I64u ", ref->base.size, ref->offset);

		for (uint32_t i = 0; i < ref->base.size; i++)
		{
			printf(" ptr %d: %p", i, ref->data[i]);
		}
		return;
	}
	case TYPEID_INLINE:
	case TYPEID_REMOVED:
		return;
	default:
		break;
	}

	for (int32_t i = 0; i < elem->size; i++)
	{
		printf(" ");

		switch (elem->rawInfo.type)
		{
		case TYPEID_REAL32: // 10
			printf("%f", ((TElementFloat*)elem)->value[i]);
			break;
		case TYPEID_INT8: // 11
		case TYPEID_BINORMALINT8: // 13
			printf("%d", ((TElementInt8*)elem)->value[i]);
			break;
		case TYPEID_INT16: // 15
		case TYPEID_BINORMALINT16: // 17
			printf("%d", ((TElementInt16*)elem)->value[i]);
			break;
		case TYPEID_UINT16: // 16
		case TYPEID_NORMALUINT16: // 18
		case TYPEID_REAL16: // 21
			printf("%u", ((TElementUint16*)elem)->value[i]);
			break;
		case TYPEID_UINT8: // 12
		case TYPEID_NORMALUINT8: // 14
			printf("%u", ((TElementUint8*)elem)->value[i]);
			break;
		case TYPEID_INT32: // 19
			printf("%d", ((TElementInt32*)elem)->value[i]);
			break;
		case TYPEID_UINT32: // 20
			printf("%u", ((TElementUint32*)elem)->value[i]);
			break;
		case TYPEID_TRANSFORM: // 9
		{
			TTransformation tr = ((TElementTransform*)elem)->value[i];
			printf("flags %u x %f y %f z %f rotationx %f rotationy %f rotationz %f rotationw %f scaleshear %f %f %f %f %f %f %f %f %f", tr.flags, tr.translation[0], tr.translation[1], tr.translation[2], tr.rotation[0], tr.rotation[1], tr.rotation[2], tr.translation[3],
				tr.scaleShear[0][0], tr.scaleShear[0][1], tr.scaleShear[0][2], tr.scaleShear[1][0], tr.scaleShear[1][1], tr.scaleShear[1][2], tr.scaleShear[2][0], tr.scaleShear[2][1], tr.scaleShear[2][2]);
			break;
		}
		}
	}
}

#endif // _DEBUG