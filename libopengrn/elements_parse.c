/*!
	Project: libopengrn
	File: elements_parse.c
	Setup element nodes

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "elements.h"
#include "debug.h"
#include "typeinfo.h"
#include "virtual_ptr.h"

#include <stdlib.h>

#define TYPE_ELEMENT(type, ctype) ((type*)elem)->value = (ctype*)(data + ofs); \
								  ofs += sizeof(ctype) * elem->size;

bool Element_ParsePrimitive(TDArray* vptr, TElementGeneric* elem, const uint8_t* data, uint64_t* offset, bool b64)
{
	uint32_t ofs = *offset;

	switch (elem->rawInfo.type)
	{
	default:
		dbg_printf("unknown type: %d\n", elem->rawInfo.type);
		break;

	case TYPEID_INLINE: // 1
	case TYPEID_NONE: // 0
		return true;

	case TYPEID_INT16: // 15
	case TYPEID_BINORMALINT16: // 17
		TYPE_ELEMENT(TElementInt16, int16_t);
		break;

	case TYPEID_UINT16: // 16
	case TYPEID_NORMALUINT16: // 18
	case TYPEID_REAL16: // 21
		TYPE_ELEMENT(TElementUint16, uint16_t);
		break;

	case TYPEID_INT32: // 19
		TYPE_ELEMENT(TElementInt32, int32_t);
		break;

	case TYPEID_UINT32: // 20
		TYPE_ELEMENT(TElementUint32, uint32_t);
		break;

	case TYPEID_INT8: // 11
	case TYPEID_BINORMALINT8: // 13
		TYPE_ELEMENT(TElementInt8, int8_t);
		break;

	case TYPEID_UINT8: // 12
	case TYPEID_NORMALUINT8: // 14
		TYPE_ELEMENT(TElementUint8, uint8_t);
		break;

	case TYPEID_REAL32: // 10
		TYPE_ELEMENT(TElementFloat, float);
		break;

	case TYPEID_TRANSFORM: // 9
		TYPE_ELEMENT(TElementTransform, TTransformation);
		break;

	case TYPEID_REFERENCE: // 2
	case TYPEID_EMPTYREFERENCE: // 22
		if (b64)
		{
			((TElementReference*)elem)->reference = decode_ptr(vptr, *(uint64_t*)(data + ofs));
			ofs += 8;
		}
		else
		{
			((TElementReference*)elem)->reference = decode_ptr(vptr , *(uint32_t*)(data + ofs));
			ofs += 4;
		}

		break;

	case TYPEID_STRING: // 8
	{
		char* ptr;

		if (b64)
		{
			((TElementString*)elem)->value = (char*)decode_ptr(vptr, *(uint64_t*)(data + ofs));
			ofs += 8;
		}
		else
		{
			((TElementString*)elem)->value = (char*)decode_ptr(vptr, *(uint32_t*)(data + ofs));
			ofs += 4;
		}

		break;
	}

	case TYPEID_REFERENCETOARRAY: // 3
		elem->size = *(uint32_t*)(data + ofs);
		ofs += 4;

		if (b64)
		{
			((TElementArray*)elem)->data = decode_ptr(vptr, *(uint64_t*)(data + ofs));
			ofs += 8;
		}
		else
		{
			((TElementArray*)elem)->data = decode_ptr(vptr , *(uint32_t*)(data + ofs));
			ofs += 4;
		}

		break;

	case TYPEID_REFERENCETOVARIANTARRAY: // 7
		if (b64)
		{
			((TElementArray*)elem)->offset = *(uint64_t*)(data + ofs);
			ofs += 8;
			elem->size = *(uint32_t*)(data + ofs);
			ofs += 4;
			((TElementArray*)elem)->data = (void**)decode_ptr(vptr, *(uint64_t*)(data + ofs));;
			ofs += 8;
		}
		else
		{
			((TElementArray*)elem)->offset = *(uint32_t*)(data + ofs);
			ofs += 4;
			elem->size = *(uint32_t*)(data + ofs);
			ofs += 4;
			((TElementArray*)elem)->data = (void**)decode_ptr(vptr, *(uint32_t*)(data + ofs));
			ofs += 4;
		}

		break;

	case TYPEID_VARIANTREFERENCE: // 5
		if (b64)
		{
			((TElementArray*)elem)->offset = *(uint64_t*)(data + ofs);
			ofs += 8;
			((TElementArray*)elem)->data = (void**)decode_ptr(vptr, *(uint64_t*)(data + ofs));
			ofs += 8;
		}
		else
		{
			((TElementArray*)elem)->offset = *(uint32_t*)(data + ofs);
			ofs += 4;
			((TElementArray*)elem)->data = (void**)decode_ptr(vptr, *(uint32_t*)(data + ofs));
			ofs += 4;
		}

		break;

	case TYPEID_ARRAYOFREFERENCES: // 4
		elem->size = *(uint32_t*)(data + ofs);
		ofs += 4;

        ((TElementArray*)elem)->data = malloc(sizeof(void*) * elem->size);

		if (b64)
		{
			((TElementArray*)elem)->offset = (uint64_t)decode_ptr(vptr, *(uint64_t*)(data + ofs));
			ofs += 8;
		}
		else
		{
			((TElementArray*)elem)->offset = (uint64_t)decode_ptr(vptr, *(uint32_t*)(data + ofs));
			ofs += 4;
		}

		for (uint32_t i = 0; i < elem->size; i++)
		{
			TElementArray* e2 = (TElementArray*)elem;

			if (b64)
			{
				((TElementArray*)e2)->data[i] = decode_ptr(vptr, *(uint64_t*)(e2->offset + (i * 8)));
			}
			else
			{
				((TElementArray*)e2)->data[i] = decode_ptr(vptr, *(uint32_t*)(e2->offset + (i*4)));
			}
		}

		break;
	}

	*offset = ofs;
	return true;
}


static bool Element_ParseNode(TDArray* vptr, TElementGeneric* elem, bool is64, TDArray* global, const uint8_t* data, uint64_t* rootOffset)
{
	// Parse current element
	uint64_t newRootOffset = 0;
	const uint8_t* typeRoot = (uint8_t*)decode_ptr(vptr, elem->rawInfo.childrenOffset);

	if (!typeRoot)
		return true;

	if (elem->rawInfo.type == TYPEID_REFERENCE || elem->rawInfo.type == TYPEID_EMPTYREFERENCE || elem->rawInfo.type == TYPEID_VARIANTREFERENCE)
	{
		TElementReference* ref = (TElementReference*)elem;
		if (!ref->reference)
			return true;

		return Element_Parse(vptr, typeRoot, (const uint8_t*)ref->reference + ref->offset, is64, global, elem, &newRootOffset);
	}
	else if (elem->rawInfo.type == TYPEID_REFERENCETOARRAY || elem->rawInfo.type == TYPEID_REFERENCETOVARIANTARRAY)
	{
		TElementArray* ref = (TElementArray*)elem;
		if (!ref->data)
			return true;

		return Element_Parse(vptr, typeRoot, (const uint8_t*)ref->data + ref->offset, is64, global, elem, &newRootOffset);
	}
	else if (elem->rawInfo.type == TYPEID_ARRAYOFREFERENCES)
	{
		TElementArray* ref = (TElementArray*)elem;

		for (uint32_t i = 0; i < ref->base.size; i++)
		{
			newRootOffset = 0;

			if (!ref->data[i])
				return true;

			if (!Element_Parse(vptr, typeRoot, (const uint8_t*)ref->data[i], is64, global, elem, &newRootOffset))
				return false;
		}
	}
	else if (elem->rawInfo.type == TYPEID_INLINE)
	{
		return Element_Parse(vptr, typeRoot, data, is64, global, elem, rootOffset);
	}

	return true;
}

bool Element_Parse(TDArray* vptr, const uint8_t* type, const uint8_t* data, bool is64, TDArray* global, TElementGeneric* parent, uint64_t* rootOffset)
{
	uint64_t offset = 0;
	TNodeTypeInfo elem;
	TElementGeneric* newElement;

	dbg_printf("enter element parse %p %p parent %s", type, data, parent->name);

	while (TypeInfo_Parse(type, &elem, is64, &offset))
	{

		newElement = Element_CreateFromTypeInfo(vptr, &elem);
		if (!newElement)
			return false;

		if (!Element_ParsePrimitive(vptr, newElement, data, rootOffset, is64))
		{
			dbg_printf("cannot parse element %p %p %zu", type, data, elem.nameOffset);
			Element_Free(&newElement);
			return false;
		}

#ifdef _DEBUG
		dbg_printf2("parsed %s type %u offset %I64u root %I64u datasize %u values", newElement->name, elem.type, offset, *rootOffset, newElement->size);
		dbg_printelement(newElement);
		dbg_printf3("\n");
#endif // _DEBUG

		if (elem.type >= TYPEID_INLINE && elem.type <= TYPEID_REFERENCETOVARIANTARRAY && elem.type != TYPEID_REMOVED)
		{
			if (!Element_ParseNode(vptr, newElement, is64, global, data, rootOffset))
				return false;
		}

		// global element array, to get all elements via a list
		if (!DArray_Add(global, &newElement))
			return false;

		if (!DArray_Add(&parent->children, &newElement))
			return false;
	}

	return true;
}
