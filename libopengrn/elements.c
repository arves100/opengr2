/*!
	Project: libopengrn
	File: elements.c
	Implementation of Gr2 element API

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "elements.h"
#include "debug.h"
#include "platform.h"
#include "virtual_ptr.h"
#include "typeinfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MALLOC_ELEMENT(type) elem = (TElementGeneric*)malloc(sizeof(type)); \
									if (!elem) \
										return NULL; \
									elem->size = info->arraySize; \
									if (!elem->size) \
										elem->size = 1; \
									((type*)elem)->value = NULL;


static bool Element_CanHaveChildren(uint32_t type)
{
	return type == TYPEID_REFERENCETOARRAY || type == TYPEID_INLINE || type == TYPEID_ARRAYOFREFERENCES || type == TYPEID_REFERENCETOVARIANTARRAY ||
		type == TYPEID_VARIANTREFERENCE || type == TYPEID_REFERENCE;
}

/*
	Checks if the element does or does not have an array, some elements can't have an array size therwise it's note will be invalid.
	@param type Type of node to check
	@param size Size of the node

	@note Data without array size: (array size MUST BE 0 or the node is invalid)
	- Reference
	- ReferenceToArray
	- ArrayOfReferences
	- VariantReference
	- ReferenceToVariantArray
	- String
	- EmptyReference
*/
static bool Element_IsArrayValid(uint32_t type, int32_t size)
{
	if (type == TYPEID_REFERENCE || type == TYPEID_REFERENCETOARRAY || type == TYPEID_ARRAYOFREFERENCES || type == TYPEID_VARIANTREFERENCE
		|| type == TYPEID_REFERENCETOVARIANTARRAY || type == TYPEID_EMPTYREFERENCE || type == TYPEID_STRING)
		return size == 0; // string, references and gr2 arrays must have size=0 (as the TypeNode info size is the one of the primitive type)

	return true;
}

void OG_DLLAPI Element_Free(TElementGeneric** elem)
{
	if (!*elem)
		return;

	if ((*elem)->rawInfo.type == TYPEID_ARRAYOFREFERENCES)
		free(((TElementArray*)(*elem))->data);

	for (size_t i = 0; i < (*elem)->children.count; i++)
	{
		// free all children
		Element_Free((TElementGeneric**)DArray_Get(&(*elem)->children, i));
	}

	DArray_Free(&(*elem)->children);
	free(*elem);
	elem = NULL;
}


TElementGeneric* Element_CreateFromTypeInfo(TDArray* vptr, TNodeTypeInfo* info)
{
	TElementGeneric* elem;

	if (!Element_IsArrayValid(info->type, info->arraySize))
		return NULL;

	switch (info->type)
	{
	case TYPEID_INT16: // 15
	case TYPEID_BINORMALINT16: // 17
		MALLOC_ELEMENT(TElementInt16);
		break;

	case TYPEID_UINT16: // 16
	case TYPEID_NORMALUINT16: // 18
	case TYPEID_REAL16: // 21
		MALLOC_ELEMENT(TElementUint16);
		break;

	case TYPEID_INT32: // 19
		MALLOC_ELEMENT(TElementInt32);
		break;

	case TYPEID_UINT32: // 20
		MALLOC_ELEMENT(TElementUint32);
		break;

	case TYPEID_BINORMALINT8: // 13
	case TYPEID_INT8: // 11
		MALLOC_ELEMENT(TElementInt8);
		break;

	case TYPEID_NORMALUINT8: // 14
	case TYPEID_UINT8: // 12
		MALLOC_ELEMENT(TElementUint8);
		break;

	case TYPEID_REAL32: // 10
		MALLOC_ELEMENT(TElementFloat);
		break;

	case TYPEID_TRANSFORM: // 9
		MALLOC_ELEMENT(TElementTransform);
		break;

	case TYPEID_STRING: // 8
		elem = (TElementGeneric*)malloc(sizeof(TElementString));
		if (!elem)
			return NULL;

		elem->size = 1;
		((TElementString*)elem)->value = NULL;
		break;

	case TYPEID_REFERENCE: // 2
	case TYPEID_EMPTYREFERENCE: // 22
		elem = (TElementGeneric*)malloc(sizeof(TElementReference));
		if (!elem)
			return NULL;

		elem->size = 1;
		((TElementReference*)elem)->offset = 0;
		((TElementReference*)elem)->reference = NULL;
		break;

	case TYPEID_REFERENCETOARRAY: // 3
	case TYPEID_VARIANTREFERENCE: // 5
	case TYPEID_ARRAYOFREFERENCES: // 4
	case TYPEID_REFERENCETOVARIANTARRAY: // 7
		elem = (TElementGeneric*)malloc(sizeof(TElementArray));
		if (!elem)
			return NULL;

		elem->size = 1;

        ((TElementArray*)elem)->data = NULL;
		((TElementArray*)elem)->offset = 0;
		break;

	case TYPEID_INLINE: // 1
		elem = (TElementGeneric*)malloc(sizeof(TElementGeneric));
		if (!elem)
			return NULL;

		elem->size = 1;
		break;

	case TYPEID_NONE: // 0
		return NULL;

	default:
		dbg_printf("Invalid type %u\n", info->type);
		return NULL;
	}

	if (!elem)
		return NULL;

	elem->rawInfo = *info;

	if (info->nameOffset)
		elem->name = decode_ptr(vptr, info->nameOffset);
	else
		elem->name = NULL;

	if (!DArray_Init(&elem->children, sizeof(TElementGeneric*), Element_CanHaveChildren(elem->rawInfo.type) ? 1 : 0))
	{
		free(elem);
		return NULL;
	}

	return elem;
}

bool Element_New(uint32_t type, const char* name, TElementGeneric** out)
{
	TNodeTypeInfo info;
	TElementGeneric* elem;

	memset(&info, 0, sizeof(info));
	info.type = type;

	elem = Element_CreateFromTypeInfo(NULL, &info); // we can safetly pass NULL here because nameOffset won't be setted up

	if (!elem)
		return false;

	elem->name = name;
	*out = elem;
	return true;
}
