/*!
	Project: libopengrn
	File: elements.h
	Definition of Gr2 element API

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#pragma once

#include "darray.h"
#include "structures.h"

/*!
	Information about element types
*/
typedef struct STypeInfo
{
	uint32_t size32; /* size of the element in 32bit mode */
	uint32_t size64; /* size of the element in 64bit mode */
	uint32_t swapSize; /* size for swap type in marshalling fix */
} TTypeInfo;

/*!
	Gr2 element types
*/
enum TypeIDs
{
	TYPEID_NONE = 0,
	TYPEID_INLINE = 1, // Empty node with just childrens
	TYPEID_REFERENCE = 2, // Reference to a pointer
	TYPEID_REFERENCETOARRAY = 3, // Reference to an array
	TYPEID_ARRAYOFREFERENCES = 4, // Array containing a numbers of pointers
	TYPEID_VARIANTREFERENCE = 5, // Reference with offset
	TYPEID_REMOVED = 6, // TODO: We know this was used to be reference or a custom type, is there anything that reference this?
	TYPEID_REFERENCETOVARIANTARRAY = 7, // Reference to an array with offset
	TYPEID_STRING = 8,
	TYPEID_TRANSFORM = 9,
	TYPEID_REAL32 = 10,
	TYPEID_INT8 = 11,
	TYPEID_UINT8 = 12,
	TYPEID_BINORMALINT8 = 13, // TODO: discover what changes between this and int8
	TYPEID_NORMALUINT8 = 14, // TODO: discover what changes between this and uint8
	TYPEID_INT16 = 15,
	TYPEID_UINT16 = 16,
	TYPEID_BINORMALINT16 = 17, // TODO: discover what changes between this and int16
	TYPEID_NORMALUINT16 = 18, // TODO: discover what changes between this and uint16
	TYPEID_INT32 = 19,
	TYPEID_UINT32 = 20,
	TYPEID_REAL16 = 21, // half-sized floating value
	TYPEID_EMPTYREFERENCE = 22,
	// End of known elements as of Granny 2.12.0.2 (mostly this will be all)

	TYPEID_MAX,
};

/*!
	Global holder of element type info
*/
extern OG_DLLAPI TTypeInfo ELEMENT_TYPE_INFO[23];

/*!
	Gr2 element (like ArtToolInfo)
*/
typedef struct SElementInfo
{
	TNodeTypeInfo info;
	const char* name; /* name */
	TDArray children; /* childrens */

	/*
		pointer to data

		Possible values:
			Type 3, 4, 5, 7: pointer to its structure
			Type 8: pointer to a string
			Type 0, 1: NULL
			Type 2, 22: pointer
			other types: new type[1...n]
	*/
	void* data;
	uint64_t dataSize; /* complete size of the data */
} TElementInfo;

/*!
	Used in Type3
*/
typedef struct SReferenceToArrayData
{
	uint32_t size; /* length of the array */
	void* data; /* pointer size 32/64 */
} TReferenceToArrayData;

/*!
	Used in Type7
*/
typedef struct SReferenceToVariantArrayData
{
	uint64_t offset; /* position of the array */
	uint32_t size; /* length of the array */
	void* data; /* pointer size 32/64 */
} TReferenceToVariantArrayData;

/*!
	Used in Type5
*/
typedef struct SVariantReference
{
	uint64_t offset; /* where the pointer is */
	void* data; /* position of the reference */
} TVariantReference;

/*!
	Used on Type4
*/
typedef struct SArrayReferenceData
{
	uint32_t size; /* length of the array */
	uint64_t offset; /* location of reference info */
	void** ptr; /* array of pointers */
} TArrayReferenceData;

extern OG_DLLAPI bool Element_New(uint32_t type, const char* name, TElementInfo* out);
extern OG_DLLAPI void Element_Free(TElementInfo* elem);

extern OG_DLLAPI bool Element_Parse(const uint8_t* type, const uint8_t* data, bool is64, TDArray* da, TElementInfo* parent);

extern OG_DLLAPI bool Element_SetPrimitiveData(TElementInfo* elem, const uint8_t* data);
extern OG_DLLAPI bool Element_AddToChild(TElementInfo* elem, size_t arrayPos);

extern OG_DLLAPI bool Element_CreateArray(TElementInfo* elem, size_t len);
extern OG_DLLAPI bool Element_AddToArray(TElementInfo* elem, size_t pos, void* data);

