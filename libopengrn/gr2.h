/*!
	Project: libopengrn
	File: gr2.h
	Low Level API for Gr2 files

	Copyright (C) 2021 Arves100
*/
#pragma once

#include <stdbool.h>
#include "elements.h"
#include "structures.h"
#include "darray.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct SGr2
{
	/* for fixing data...*/

	bool mismatchEndianness; /* if the file and platform mismatches endianness */
	bool mismatchBits /* if the file and platform mismatches bits (like file is 32-bit and app is 64-bit) */;

	THeader header; /* gr2 header */
	TFileInfo fileInfo; /* gr2 file info */
	TSector* sectors; /* gr2 sectors info */

	uint8_t* data; /* full decompressed data of the file */
	size_t* sectorOffsets; /* offsets of gr2 sectors */
	size_t dataSize; /* full size of the data */

	TElementInfo root; /* root element */
	TDArray elements; /* all elements of the gr2 (sizeof(TNodeTypeInfo)) */
} TGr2;

extern bool OG_DLLAPI Gr2_Init(TGr2* gr2);
extern void OG_DLLAPI Gr2_Free(TGr2* gr2);

extern bool OG_DLLAPI Gr2_Load(const uint8_t* src, size_t len, TGr2* gr2);
extern bool OG_DLLAPI Gr2_Compose(TGr2* gr2);

extern void OG_DLLAPI Gr2_SetDefaultInfo(TGr2* gr2, bool is64, bool isBe, uint32_t fileFormat);

extern bool OG_DLLAPI Gr2_AddElement(TGr2* gr2, TElementInfo* elem, bool addToRoot, size_t* posOut);

#ifdef __cplusplus
}
#endif