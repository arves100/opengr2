/*!
	Project: libopengrn
	File: gr2.c
	Low Level API for Gr2 files

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "gr2.h"
#include "debug.h"
#include "magic.h"
#include "typeinfo.h"
#include "elements.h"
#include <stdlib.h>

OG_DLLAPI bool Gr2_Init(TGr2* gr2)
{
	memset(gr2, 0, sizeof(TGr2));

	if (!DArray_Init(&gr2->virtual_ptr, sizeof(void*), 100))
		return false;

	if (!Element_New(TYPEID_INLINE, "Root", &gr2->root))
		return false;

	return DArray_Init(&gr2->elements, sizeof(TElementGeneric*), 15);
}

OG_DLLAPI void Gr2_Free(TGr2* gr2)
{
	Element_Free(&gr2->root);

	/*for (size_t i = 0; i < gr2->elements.count; i++)
	{
		Element_Free((TElementGeneric**)DArray_Get(&gr2->elements, i));
	}*/

	DArray_Free(&gr2->elements);

	if (gr2->sectorOffsets)
	{
		free(gr2->sectorOffsets);
		gr2->sectorOffsets = NULL;
	}

	if (gr2->data)
	{
		free(gr2->data);
		gr2->data = NULL;
	}

	if (gr2->sectors)
	{
		free(gr2->sectors);
		gr2->sectors = NULL;
	}

	gr2->dataSize = 0;

	DArray_Free(&gr2->virtual_ptr);
}

void OG_DLLAPI Gr2_SetDefaultInfo(TGr2* gr2, bool is64, bool isBe, uint32_t fileFormat)
{
	uint8_t magicFlags = 0;

	if (fileFormat == 7)
	{
		gr2->header.sizeWithSectors = 72;
		magicFlags |= MAGIC_FLAG_EXTRA16;
	}
	else
	{
		gr2->header.sizeWithSectors = 56;
	}

	if (is64)
		magicFlags |= MAGIC_FLAG_64BIT;

	if (isBe)
		magicFlags |= MAGIC_FLAG_BIGENDIAN;

	gr2->header.format = 0;
	Magic_Set(gr2->header.magic, magicFlags);
	memset(gr2->header.extra, 0, sizeof(gr2->header.extra));

	gr2->fileInfo.format = fileFormat;
	gr2->fileInfo.totalSize = 0;
	gr2->fileInfo.crc32 = 0;
	gr2->fileInfo.sectorCount = 8;
	gr2->fileInfo.tag = 0x80000000;
	gr2->fileInfo.type.position = 0;
	gr2->fileInfo.type.sector = 0;
	gr2->fileInfo.root.position = 0;
	gr2->fileInfo.root.sector = 0;
	memset(gr2->fileInfo.extra, 0, sizeof(gr2->fileInfo.extra));
}

TElementGeneric* OG_DLLAPI Gr2_AddElement(TGr2* gr2, uint8_t type, const char* name, TElementGeneric* root)
{
	TElementGeneric* g;

	if (!root)
		root = gr2->root;

	if (!Element_New(type, name, &g))
		return NULL;

	if (!DArray_Add(&gr2->elements, &g))
	{
		Element_Free(&g);
		return NULL;
	}

	if (!DArray_Add(&root->children, &g))
	{
		Element_Free(&g);
		return NULL;
	}

	return g;
}
