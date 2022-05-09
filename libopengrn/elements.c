/*!
	Project: libopengrn
	File: elements.c
	Implementation of Gr2 element API

	Copyright (C) 2021 Arves100
*/
#include "elements.h"
#include "platform.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

OG_DLLAPI TTypeInfo ELEMENT_TYPE_INFO[23] =
{
	{ 0, 0, 0 }, // NULL
	{ 0, 0, 0 }, // NULL
	{ 4, 8, 0 }, // void*
	{ 8, 12, 4 }, //void**
	{ 8, 12, 4 }, // void***
	{ 8, 16, 0 }, // void*
	{ 0, 0, 0, }, // __REMOVED
	{ 12, 20, 4 }, // void*
	{ 4, 8, 4, }, // char*
	{ 68, 68, 4, }, // gr2_transform
	{ 4, 4, 4, }, // float
	{ 1, 1, 1, }, // int8_t
	{ 1, 1, 1, }, // uint8_t
	{ 1, 1, 1, }, // int8_t
	{ 1, 1, 1, }, // uint8_t
	{ 2, 2, 2, }, // int16_t
	{ 2, 2, 2, }, // uint16_t
	{ 2, 2, 2, }, // int16_t
	{ 2, 2, 2, }, // uint16_t
	{ 4, 4, 4, }, // int32_t
	{ 4, 4, 4, }, // uint32_t
	{ 2, 2, 2, }, // float/2
	{ 4, 8, 0, }, // void*
};

/*
	Inline node does not have a size, it's a container to various node

	Data without array size: (array size MUST BE 0 or the node is invalid)
	Reference
	ReferenceToArray
	ArrayOfReferences
	VariantReference
	ReferenceToVariantArray
	String
	EmptyReference

	If the array size is 0 for all the other elements default it to 1
*/
static bool Element_IsArrayValid(uint32_t type, int32_t size)
{
	if (type == TYPEID_REFERENCE || type == TYPEID_REFERENCETOARRAY || type == TYPEID_ARRAYOFREFERENCES || type == TYPEID_VARIANTREFERENCE
		|| type == TYPEID_REFERENCETOVARIANTARRAY || type == TYPEID_EMPTYREFERENCE || type == TYPEID_STRING)
		return size == 0;

	return true;
}

static bool Element_SetPrimitiveDataWithOffset(TElementInfo* elem, const uint8_t* data, uint64_t* offset)
{
	size_t elementSize;
	int32_t arraySize = elem->info.arraySize;

	if (!arraySize)
		arraySize = 1;

	/*
	if (Platform_GetPointerSize() == 8)
		elementSize = ELEMENT_TYPE_INFO[elem->info.type].size64;
	else*/
	elementSize = ELEMENT_TYPE_INFO[elem->info.type].size32;

	elem->dataSize = elementSize * arraySize;
	elem->data = malloc((size_t)elem->dataSize);

	if (!elem->data)
	{
		dbg_printf("memory allocation fail!!!");
		return false;
	}

	for (int32_t i = 0; i < arraySize; i++)
	{
		/*
			32-bit and 64-bit sizes are the same for primitives, but this code is left in case
			a future primitive that has mismatched size exists

			if (is64)
				memcpy_s(((uint8_t*)elem->data) + (i * elementSize), elementSize, data + (i * elementSize), ELEMENT_TYPE_INFO[elem->info.type].size64);
			else
		*/
		memcpy_s(((uint8_t*)elem->data) + (i * elementSize), elementSize, data + (i * elementSize), ELEMENT_TYPE_INFO[elem->info.type].size32);
	}

	if (offset)
		*offset += elem->dataSize;
	return true;
}

static bool Element_SetData(TElementInfo* elem, const uint8_t* data, bool is64, uint64_t* offset)
{
	if (!Element_IsArrayValid(elem->info.type, elem->info.arraySize))
	{
		dbg_printf("invalid size %d for element", elem->info.arraySize);
		return false;
	}

	elem->name = (char*)elem->info.nameOffset;
	elem->dataSize = 0;
	
	if (!DArray_Init(&elem->children, sizeof(size_t), 3))
		return false;

	switch (elem->info.type) // types that do require manual handling
	{
	case TYPEID_REFERENCE: // 2
	case TYPEID_EMPTYREFERENCE: // 22
		if (is64)
		{
			elem->data = (void*)*(uint64_t*)data;
			*offset += ELEMENT_TYPE_INFO[elem->info.type].size64;
		}
		else
		{
			elem->data = (void*)*(uint32_t*)data;
			*offset += ELEMENT_TYPE_INFO[elem->info.type].size32;
		}

		return true;
	case TYPEID_INLINE: // 1
		elem->data = NULL;
		return true;
	case TYPEID_STRING: // 8
	{
		char* ptr;

		if (is64)
		{
			ptr = (char*)*(uint64_t*)data;
			*offset += ELEMENT_TYPE_INFO[elem->info.type].size64;
		}
		else
		{
			ptr = (char*)*(uint32_t*)data;
			*offset += ELEMENT_TYPE_INFO[elem->info.type].size32;
		}

		elem->dataSize = strlen(ptr) + 1;
		elem->data = (void*)malloc((size_t)elem->dataSize);

		if (!elem->data)
		{
			dbg_printf("memory allocation fail!!!");
			return false;
		}

		memset(elem->data, 0, (size_t)elem->dataSize);
		memcpy_s(elem->data, (size_t)elem->dataSize, ptr, elem->dataSize - 1);

		return true;
	}
	case TYPEID_ARRAYOFREFERENCES: // 4
	{
		TArrayReferenceData* ref = (TArrayReferenceData*)malloc(sizeof(TArrayReferenceData));

		if (!ref)
		{
			dbg_printf("memory allocation fail!!!");
			return false;
		}

		ref->size = *(uint32_t*)data;

		if (is64)
		{
			ref->offset = * (uint64_t*)(data + 4);
			*offset += ELEMENT_TYPE_INFO[elem->info.type].size64;

		}
		else
		{
			ref->offset = * (uint32_t*)(data + 4);
			*offset += ELEMENT_TYPE_INFO[elem->info.type].size32;
		}

		ref->ptr = malloc(ref->size * sizeof(void*));
		if (!ref->ptr)
		{
			dbg_printf("memory allocation fail!!!");
			free(ref);
			return false;
		}

		for (uint32_t i = 0; i < ref->size; i++)
		{
			uint8_t* dst = ((uint8_t*)ref->ptr) + (i * sizeof(void*));
			uint64_t src;

			if (is64)
			{
				src = *(uint64_t*)(ref->offset + (sizeof(void*) * i));
			}
			else
				src = *(uint32_t*)(ref->offset + (sizeof(void*) * i));

			memcpy_s(dst, sizeof(void*), &src, sizeof(void*));
		}

		elem->dataSize = (ref->size * sizeof(void*)) + sizeof(*ref);
		elem->data = ref;
		return true;
	}
	case TYPEID_REFERENCETOARRAY: // 3
	{
		TReferenceToArrayData* ref = (TReferenceToArrayData*)malloc(sizeof(TReferenceToArrayData));

		if (!ref)
		{
			dbg_printf("memory allocation fail!!!");
			return false;
		}

		ref->size = *(uint32_t*)data;

		if (is64)
		{
			ref->data = (void*)*(uint64_t*)(data + 4);

			*offset += ELEMENT_TYPE_INFO[elem->info.type].size64;
		}
		else
		{
			ref->data = (void*)*(uint32_t*)(data + 4);

			*offset += ELEMENT_TYPE_INFO[elem->info.type].size32;
		}

		elem->dataSize = sizeof(*ref);
		elem->data = ref;
		return true;
	}
	case TYPEID_REFERENCETOVARIANTARRAY: // 7
	{
		TReferenceToVariantArrayData* ref = (TReferenceToVariantArrayData*)malloc(sizeof(TReferenceToVariantArrayData));

		if (!ref)
		{
			dbg_printf("memory allocation fail!!!");
			return false;
		}

		if (is64)
		{
			ref->offset = *(uint64_t*)(data);
			ref->size = *(uint32_t*)(data + 8);
			ref->data = (void*)*(uint64_t*)(data + 12);

			*offset += ELEMENT_TYPE_INFO[elem->info.type].size64;
		}
		else
		{
			ref->offset = *(uint32_t*)data;
			ref->size = *(uint32_t*)(data + 4);
			ref->data = (void*)*(uint32_t*)(data + 8);

			*offset += ELEMENT_TYPE_INFO[elem->info.type].size32;
		}

		elem->dataSize = sizeof(*ref);
		elem->data = ref;
		return true;
	}
	case TYPEID_VARIANTREFERENCE: // 5
	{
		TVariantReference* ref = (TVariantReference*)malloc(sizeof(TVariantReference));

		if (!ref)
		{
			dbg_printf("memory allocation fail!!!");
			return false;
		}

		if (is64)
		{
			ref->offset = *(uint64_t*)data;
			ref->data = (void*)*(uint64_t*)(data + 8);

			*offset += ELEMENT_TYPE_INFO[elem->info.type].size64;
		}
		else
		{
			ref->offset = *(uint32_t*)data;
			ref->data = (void*)*(uint32_t*)(data + 4);

			*offset += ELEMENT_TYPE_INFO[elem->info.type].size32;
		}

		elem->dataSize = sizeof(*ref);
		elem->data = ref;
		return true;
	}
	default:
		break;
	}

	return Element_SetPrimitiveDataWithOffset(elem, data, offset);
}

static bool Element_ParseInfo(const uint8_t* data, TNodeTypeInfo* info, bool is64, uint64_t* offset)
{
	TNodeTypeInfo ni;

	ni.type = *(uint32_t*)(data + *offset);

	if (ni.type == 0 || ni.type > TYPEID_MAX)
		return false;

	*offset += 4;

	if (is64)
	{
		ni.nameOffset = *(uint64_t*)(data + *offset);
		*offset += 8;
		ni.childrenOffset = *(uint64_t*)(data + *offset);
		*offset += 16;
	}
	else
	{
		ni.nameOffset = *(uint32_t*)(data + *offset);
		*offset += 4;
		ni.childrenOffset = *(uint32_t*)(data + *offset);
		*offset += 4;
	}

	ni.arraySize = *(int32_t*)(data + *offset);

	*offset += 4;

	memcpy_s(ni.extra, 12, data + *offset, 12);
	*offset += sizeof(ni.extra);

	if (is64)
	{
		ni.extra4 = *(uint64_t*)(data + *offset);
		*offset += 8;
	}
	else
	{
		ni.extra4 = *(uint32_t*)(data + *offset);
		*offset += 4;
	}

	*info = ni;
	return true;
}

static bool Element_ParseNode(TElementInfo* elem, bool is64, TDArray* global)
{
	const uint8_t* typeRoot = (uint8_t*)elem->info.childrenOffset;

	if (!typeRoot)
		return true;

	if (elem->info.type == TYPEID_REFERENCE || elem->info.type == TYPEID_EMPTYREFERENCE)
	{
		if (!elem->data)
			return true;

		return Element_Parse(typeRoot, (uint8_t*)elem->data, is64, global, elem);
	}
	/*else if (elem->info.type == TYPEID_VARIANTREFERENCE)
	{
		TVariantReference* ref = (TVariantReference*)elem->data;

		if (!ref->data)
			return true;

		return Element_Parse(typeRoot, (uint8_t*)(ref->data) + ref->offset, is64, da, elem);
	}
	else if (elem->info.type == TYPEID_REFERENCETOARRAY)
	{
		TReferenceToArrayData* ref = (TReferenceToArrayData*)elem->data;

		if (!ref->data)
			return true;

		for (uint32_t i = 0; i < ref->size; i++)
		{

			//return Element_Parse(typeRoot, (uint8_t*)(ref->data) + (sizeof(void*) * i), is64, da, elem);
		}

	}
	else if (elem->info.type == TYPEID_REFERENCETOVARIANTARRAY)
	{
		TReferenceToVariantArrayData* ref = (TReferenceToVariantArrayData*)elem->data;

		if (!ref->data)
			return true;

		for (uint32_t i = 0; i < ref->size; i++)
		{

			//return Element_Parse(typeRoot, (uint8_t*)(ref->data) + (sizeof(void*) * i), is64, da, elem);
		}
	}*/
	else if (elem->info.type == TYPEID_ARRAYOFREFERENCES)
	{
		TArrayReferenceData* ref = (TArrayReferenceData*)elem->data;

		if (!ref->ptr)
			return true;

		for (uint32_t i = 0; i < ref->size; i++)
		{
			if (!ref->ptr[i])
				return true;

			return Element_Parse(typeRoot, ref->ptr[i], is64, global, elem);
		}
	}

	return true;
}

bool OG_DLLAPI Element_Parse(const uint8_t* type, const uint8_t* data, bool is64, TDArray* global, TElementInfo* parent)
{
	uint64_t offset = 0, rootOffset = 0;
	TElementInfo elem;
	size_t elem_pos;

	dbg_printf("enter element parse %p %p parent %s", type, data, parent->name);

	while (Element_ParseInfo(type, &elem.info, is64, &offset))
	{
		if (!Element_SetData(&elem, data + rootOffset, is64, &rootOffset))
			return false;

        printf("Element with type %i\n", elem.info.type);

#ifdef _DEBUG
		dbg_printf2("parsed %s type %u offset %I64u root %I64u datasize %I64u values", elem.name, elem.info.type, offset, rootOffset, elem.dataSize);
		dbg_printelement(&elem);
		dbg_printf3("\n");
#endif // _DEBUG

		if (elem.info.type >= TYPEID_INLINE && elem.info.type <= TYPEID_REFERENCETOVARIANTARRAY && elem.info.type != TYPEID_REMOVED)
		{
			if (!Element_ParseNode(&elem, is64, global))
				return false;
		}

		if (!DArray_Add(global, &elem))
			return false;

		elem_pos = global->count - 1;

		if (!DArray_Add(&parent->children, &elem_pos))
			return false;
	}

	return true;
}

bool OG_DLLAPI Element_New(uint32_t type, const char* name, TElementInfo* out)
{
	out->data = NULL;
	out->dataSize = 0;
	out->name = name;

	out->info.arraySize = 0;
	out->info.childrenOffset = 0;
	out->info.type = type;
	out->info.nameOffset = 0;
	out->info.extra4 = 0;
	memset(out->info.extra, 0, sizeof(out->info.extra));

	return DArray_Init(&out->children, sizeof(size_t), 3);
}

void OG_DLLAPI Element_Free(TElementInfo* elem)
{
	DArray_Free(&elem->children);
	
	if (elem->data && elem->info.type != TYPEID_REFERENCE && elem->info.type != TYPEID_EMPTYREFERENCE)
	{
		if (elem->info.type == TYPEID_ARRAYOFREFERENCES)
		{
			TArrayReferenceData* ref = (TArrayReferenceData*)elem->data;

			if (ref->ptr)
				free(ref->ptr);
		}

		free(elem->data);
	}

	elem->data = NULL;
	elem->dataSize = 0;
}

OG_DLLAPI bool Element_SetPrimitiveData(TElementInfo* elem, const uint8_t* data)
{
	if (elem->info.type == TYPEID_STRING)
	{
		elem->dataSize = strlen((const char*)data) + 1;
		elem->data = malloc((size_t)elem->dataSize);

		if (!elem->data)
		{
			dbg_printf("memory allocation fail!!!");
			return false;
		}

		memset(elem->data, 0, (size_t)elem->dataSize);
		memcpy_s(elem->data, (size_t)elem->dataSize, data, elem->dataSize - 1);
		return true;
	}

	return Element_SetPrimitiveDataWithOffset(elem, data, NULL);
}

OG_DLLAPI bool Element_AddToChild(TElementInfo* elem, size_t arrayPos)
{
	return DArray_Add(&elem->children, &arrayPos);
}

OG_DLLAPI bool Element_CreateArray(TElementInfo* elem, size_t len)
{
	elem->dataSize = len * ELEMENT_TYPE_INFO[elem->info.type].size32;
	elem->data = malloc((size_t)elem->dataSize);

	if (!elem->data)
		return false;

	memset(elem->data, 0, (size_t)elem->dataSize);
	return true;
}

OG_DLLAPI bool Element_AddToArray(TElementInfo* elem, size_t pos, void* data)
{
	if (((pos + 1) * ELEMENT_TYPE_INFO[elem->info.type].size32) > elem->dataSize)
		return false;

	//memcpy_s((char*)elem->data + (pos * ELEMENT_TYPE_INFO[elem->info.type].size32), data, ELEMENT_TYPE_INFO[elem->info.type].size32);
	return true;
}
