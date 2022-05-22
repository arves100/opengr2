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
	Gr2 generic element
*/
typedef struct SElementGeneric
{
	TNodeTypeInfo rawInfo; //! Raw node info
	const char* name; //! Node name
	TDArray children; //! Dynamic array that stores the pointers of the children
	uint32_t size; /// Size of the element array (which is also used in the number of array elements), in case of string this will determine the length
} TElementGeneric;

/*!
	Gr2 string element (TYPEID_STRING)
*/
typedef struct SElementString
{
	TElementGeneric base; //! Base element

	/*!
		Pointer to a buffer which contains a string
		@note If the length is not 0, it means that the program has allocated
			a new buffer for value, therefore it should be manually cleared, if the length
			if 0, it means that the buffer cannot be deallocated.

		@note THE LIBRARY MAKES NO CHECK WHENVER LENGTH OF VALUE ARE OK, THE PROGRAMMER
			HAS TO MANUALLY ENSURE SETTING PROPER DATA!
		@note The length is base.size
	*/
	const char* value; //! String value
} TElementString;

#define GR2_DEFINE_ELEMENT(name, type) typedef struct SElement##name { TElementGeneric base; type * value; } TElement##name;

// TYPEID_INT8
// TYPEID_BINORMALINT8
GR2_DEFINE_ELEMENT(Int8, int8_t);

// TYPEID_UINT8
// TYPEID_NORMALUINT8
GR2_DEFINE_ELEMENT(Uint8, uint8_t);

// TYPEID_INT16
// TYPEID_BINORMALINT16
GR2_DEFINE_ELEMENT(Int16, int16_t);

// TYPEID_UINT16
// TYPEID_NORMALUINT16
// TYPEID_REAL16
GR2_DEFINE_ELEMENT(Uint16, uint16_t);

// TYPEID_INT32
GR2_DEFINE_ELEMENT(Int32, int32_t);

// TYPEID_UINT32
GR2_DEFINE_ELEMENT(Uint32, uint32_t);

// TYPEID_REAL32
GR2_DEFINE_ELEMENT(Float, float);

// TYPEID_TRANSFORM
GR2_DEFINE_ELEMENT(Transform, TTransformation);

/*!
	Gr2 reference type

	Any type that is:
	- TYPEID_REFERENCE
	- TYPEID_EMPTYREFERENCE
	- TYPEID_VARIANTREFERENCE
*/
typedef struct SElementReference
{
	TElementGeneric base; /// Base element
	uint64_t offset; /// Offset from the pointer of reference, used in variant type of reference

	/*!
		Contains a pointer to a reference
		@note THIS REFERENCE IS ENCAPSULATED WITH VIRTUAL_PTRS
		KEEP IN MIND BEFORE ACCESSING OR MODIFYING
	*/
	TElementGeneric* reference;
} TElementReference;

/*!
	Gr2 array

	Any type that is:
	- TYPEID_ARRAYOFREFERENCES
	- TYPEID_REFERENCETOVARIANTARRAY
	- TYPEID_REFERENCETOARRAY
*/
typedef struct SElementArray
{
	TElementGeneric base; /// Base element
	uint64_t offset; /// Offset from the pointer of data, used in variant type of reference

	/*!
		Contains a pointer to an array

		Possible values of this:
			TYPEID_ARRAYOFREFERENCES:
				Must be casted into TElementReference**
			TYPEID_REFERENCETOVARIANTARRAY or TYPEID_REFERENCETOARRAY:
				Must be a structure with all the nodes of the children

		@note THIS REFERENCE IS ENCAPSULATED WITH VIRTUAL_PTRS
		KEEP IN MIND BEFORE ACCESSING OR MODIFYING
	*/
	void** data;
} TElementArray;

extern TElementGeneric* Element_CreateFromTypeInfo(TDArray* vptr, TNodeTypeInfo* info);
extern bool Element_Parse(TDArray* vptr, const uint8_t* type, const uint8_t* data, bool is64, TDArray* global, TElementGeneric* parent, uint64_t* rootOffset);
extern void Element_Free(TElementGeneric** elem);
extern bool Element_New(uint32_t type, const char* name, TElementGeneric** out);
