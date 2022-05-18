/*!
	Project: libopengrn
	File: gr2.h
	Low Level API for Gr2 files

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <stdbool.h>
#include "elements.h"
#include "structures.h"
#include "darray.h"

#ifdef __cplusplus
extern "C"{
#endif

/*!
	The main container of all the Granny2 informations	
*/
typedef struct SGr2
{
	bool mismatchEndianness; /* if the file and platform mismatches endianness */
	uint8_t bitsSize; /* bits size of the file */

	THeader header; /* gr2 header */
	TFileInfo fileInfo; /* gr2 file info */
	TSector* sectors; /* gr2 sectors info */

	uint8_t* data; /* full decompressed data of the file */
	size_t* sectorOffsets; /* offsets of gr2 sectors */
	size_t dataSize; /* full size of the data */

	TDArray virtual_ptr; /* virtual pointer array node */

	TElementGeneric* root; /* root element */
	TDArray elements; /* all elements of the gr2 (sizeof(TNodeTypeInfo)) */
} TGr2;

/*!
	Initializes a new Gr2 structure
	@param gr2 The structure to initialize
	@return true if the initialization succeeded, otherwise false
*/
extern bool OG_DLLAPI Gr2_Init(TGr2* gr2);

/*!
	Frees all the allocated memory of a Gr2 structure
	@param gr2 The structure to free
*/
extern void OG_DLLAPI Gr2_Free(TGr2* gr2);

/*!
	Loads a Granny2 file and stores it inside the Gr2 structure
	@param src Source data to load
	@param len Length of the data
	@param gr2 The structure to store the data
	@return true if the load succedded, otherwise false
*/
extern bool OG_DLLAPI Gr2_Load(const uint8_t* src, size_t len, TGr2* gr2);
extern bool OG_DLLAPI Gr2_Compose(TGr2* gr2);

/*!
	Sets the default information of a Gr2 structure, usefull when creating a new file
	@param gr2 The structure to set the file
	@param is64 Set this to true if you want to initialize a 64-bit file
	@param isBe Set this to true if you want to initialize a Big Endian file.
	@param fileFormat Set the file format number
*/
extern void OG_DLLAPI Gr2_SetDefaultInfo(TGr2* gr2, bool is64, bool isBe, uint32_t fileFormat);

/*!
	Adds a new element into the file
	@param gr2 The Gr2 structure to add a new element
	@param elem New element information
	@param addToRoot Set this to true if you want to add an element into the root
	@param posOut output position of the element in the dynamic array
	@return true if a new element was added, otherwise false
*/
//extern bool OG_DLLAPI Gr2_AddElement(TGr2* gr2, TElementInfo* elem, bool addToRoot, size_t* posOut);

#ifdef __cplusplus
}
#endif