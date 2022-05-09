/*!
	Project: libopengrn
	File: gr2.c
	Low Level API for Gr2 files

	Copyright (C) 2021 Arves100
*/

/*
	TODO: Check if Big Endian byte swapping is correct
	TODO: Add CRC32 support (starts from Sector info)
*/
#include "debug.h"
#include "gr2.h"
#include "magic.h"
#include "platform.h"
#include "compression.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*!
	Loads and checks the file information from the byte data
	@param gr2 The Gr2 structure to fill
	@param data The data to parse
	@param len Length of the data
	@return true if the loading succeeded, otherwise false
*/
static bool Gr2_LoadFileInfo(TGr2* gr2, const uint8_t* data, size_t len, bool extra16)
{
	uint8_t requiredSize = 0x38;

	if (extra16)
		requiredSize += 16;
	else
		memset(gr2->fileInfo.extra + 16, 0, 16);

	if (len < requiredSize + sizeof(THeader))
	{
		dbg_printf("size %zu is not enough to load header and file info");
		return false;
	}

	gr2->fileInfo = *(TFileInfo*)(data + sizeof(THeader));

	if (gr2->mismatchEndianness)
		Platform_Swap1((uint8_t*) & gr2->fileInfo, sizeof(gr2->fileInfo));

	if (gr2->fileInfo.fileInfoSize != requiredSize)
	{
		dbg_printf("requrested size %zu does not match file info size %u", requiredSize, gr2->fileInfo.fileInfoSize);
		return false;
	}

	if (gr2->fileInfo.format != 6 && gr2->fileInfo.format != 7)
	{
		dbg_printf("file format %u is not supported", gr2->fileInfo.format);
		return false;
	}

	if (len != gr2->fileInfo.totalSize)
	{
		dbg_printf("total size of file %zu mismatched requested total size %u", len, gr2->fileInfo.totalSize);
		return false;
	}

	/* TODO: crc32 calc */
	return true;
}


/*!
	Applies pointer fix ups for the gr2 content
	@param gr2 The gr2 file to fix
	@param srcSector the current sector that contains the fixup data
	@param fd fixup information
*/
static void Gr2_ApplyFixUp(TGr2* gr2, uint32_t srcSector, TFixUpData* fd)
{
	uint8_t* dst = gr2->data + gr2->sectorOffsets[fd->dstSector] + fd->dstOffset;
	uint8_t* src = gr2->data + gr2->sectorOffsets[srcSector] + fd->srcOffset;
	uint32_t dstPtr = (uint32_t)dst; /* TODO: does this work on 64bit? probably not... */

	memcpy_s(src, 4, &dstPtr, 4); /* 4 -> Pointer size on 32bit */
}

/*!
	Applies marshalling fix for endianness mismatch situations
	@param gr2 The gr2 file to fix
	@param md marshalling information
*/
static void Gr2_ApplyMarshall(TGr2* gr2, uint32_t srcSector, TMarshallData* md, const uint8_t* data)
{
	/*
		Marshalling operation: (for endianess file difference)

		1: get the node types (this will be reversed type nodes)
		2: byteswap the node type info to get it's width + nodetype (gr2 byteswap seems to ignore the last 16 bytes unk[4])
		3: get the size of all nodes (array includes)
		4: multiply element size with the array size

		case 1: the type if inline
			restart from step1 using the childOffset (swapped) as the start point
		case 2: the type is not inline and the element swap size == 4
			use Swap1 to all typearray size (array size * element size)
		case 3: the type is not inline and the element swap size != 4
			use Swap2 to all typearray size (array size * element size)

		Marshalling only applies to node info! It does not apply to node data (the data is stored in LE format)
	*/
}

OG_DLLAPI bool Gr2_Load(const uint8_t* data, size_t len, TGr2* gr2)
{
	uint32_t i;
	size_t ofs = 0;
	uint8_t magicFlags;

	/* load the magic and gr2 header */
	if (len < sizeof(THeader))
	{
		dbg_printf("file %zu is too small to have a gr2 header", len);
		return false;
	}

	if (!Magic_GetFlags((const uint32_t*)data, &magicFlags))
	{
		dbg_printf("invalid magic");
		return false;
	}

	if (Platform_GetPointerSize() != 8 && (magicFlags & MAGIC_FLAG_64BIT))
		gr2->mismatchBits = true;
	
	if (Platform_GetPointerSize() == 8 && !(magicFlags & MAGIC_FLAG_64BIT))
		gr2->mismatchBits = true;

	gr2->mismatchEndianness = Platform_IsBigEndian() != (magicFlags & MAGIC_FLAG_BIGENDIAN);

	gr2->header = *(THeader*)data;

	if (gr2->mismatchEndianness)
		Platform_Swap1((uint8_t*) & gr2->header.sizeWithSectors, 4);

	if (gr2->header.format != 0) /* header format must be 0 */
	{
		dbg_printf("header format %u is not 0", gr2->header.format);
		return false;
	}

	/* load the file info and perform the required checks */
	if (!Gr2_LoadFileInfo(gr2, data, len, magicFlags & MAGIC_FLAG_EXTRA16))
		return false;

	if (len < gr2->fileInfo.fileInfoSize + (sizeof(TSector) + gr2->fileInfo.sectorCount) + sizeof(THeader))
	{
		dbg_printf("out of bounds");
		return false;
	}


	/* allocate sector info array */
	gr2->sectors = (TSector*)malloc(sizeof(TSector) * gr2->fileInfo.sectorCount);

	if (!gr2->sectors)
	{
		dbg_printf("memory allocation fail!!!");
		return false;
	}

	/* check for sector sizes and allocate the big sector data array (where Grn nodes exists) */
	for (i = 0; i < gr2->fileInfo.sectorCount; i++)
	{
		gr2->sectors[i] = *(TSector*)(data + gr2->fileInfo.fileInfoSize + sizeof(THeader) + (i * sizeof(TSector)));

		if (gr2->sectors[i].compressType == COMPRESSION_TYPE_NONE && (gr2->sectors[i].decompressLen + gr2->sectors[i].dataOffset) > len)
		{
			dbg_printf("out of bounds");
			return false;
		}

		if (gr2->sectors[i].compressType != COMPRESSION_TYPE_NONE && (gr2->sectors[i].compressedLen + gr2->sectors[i].dataOffset) > len)
		{
			dbg_printf("out of bounds");
			return false;
		}

		gr2->dataSize += gr2->sectors[i].decompressLen;
	}

	gr2->data = (uint8_t*)malloc(gr2->dataSize);
	gr2->sectorOffsets = (size_t*)malloc(gr2->fileInfo.sectorCount * sizeof(size_t));

	if (!gr2->data || !gr2->sectorOffsets)
	{
		dbg_printf("memory allocation fail!!!");
		return false;
	}

	/* decompress the sectors and apply the required byte swapping */
	for (i = 0; i < gr2->fileInfo.sectorCount; i++)
	{
		TSector sector = gr2->sectors[i];

		if (sector.compressType == COMPRESSION_TYPE_NONE)
		{
			memcpy_s(gr2->data + ofs, gr2->dataSize - ofs, data + sector.dataOffset, sector.decompressLen);

			if (gr2->mismatchEndianness)
				Platform_Swap1(gr2->data + ofs, sector.decompressLen); /* should be done on compressed data as well */
		}
		else
		{
			// Required for Oodle
			uint32_t extraLen = Compression_GetExtraLen(sector.compressType);
			uint8_t* pComp = (uint8_t*)malloc(sector.compressedLen + extraLen);
			bool success = false;

			if (!pComp)
			{
				dbg_printf("memory allocation fail!!!");
				return false;
			}

			if (extraLen) // Required for Oodle
				memset(pComp + sector.compressedLen, 0, extraLen);

			memcpy_s(pComp, sector.compressedLen + extraLen, data + sector.dataOffset, sector.compressedLen);

			if (gr2->mismatchEndianness)
				Platform_Swap1(pComp, sector.compressedLen);

            uint8_t *pDecomp = (uint8_t*)malloc(sector.decompressLen);

			switch (sector.compressType)
			{
#if 0
			case COMPRESSION_TYPE_OODLE0:
				success = Compression_UnOodle0(pCompData, sct.compressedLen, pDecompData, cnt.info.decompressLen);
				break;
#endif
			case COMPRESSION_TYPE_OODLE1:
				success = Compression_UnOodle1(pComp, sector.compressedLen, pDecomp, sector.decompressLen, sector.oodleStop0, sector.oodleStop1);
                break;
#if 0
			case COMPRESSION_TYPE_BITKNIT1:
				success = Compression_UnBitknit1(pCompData, cnt.info.compressedLen, pDecompData, cnt.info.decompressLen);
				break;
			case COMPRESSION_TYPE_BITKNIT2:
				success = Compression_UnBitknit2(pCompData, cnt.info.compressedLen, pDecompData, cnt.info.decompressLen);
				break;
#endif
			default:
				return false;
//#endif
			}


			if (!success)
			{
				dbg_printf("decompression of %d fail", sector.compressType);
			} else {
                printf("Copy decompressed data at offset %i with length of %i (size %i)\n", ofs, sector.decompressLen, gr2->dataSize);
                memcpy_s(gr2->data + ofs, gr2->dataSize - ofs, pDecomp, sector.decompressLen);

                if (gr2->mismatchEndianness)
                    Platform_Swap1(gr2->data + ofs, sector.decompressLen); /* should be done on compressed data as well */
            }
		}

		/* must be done on decompressed data only */
		if (gr2->mismatchEndianness)
		{
			Platform_Swap1(gr2->data + ofs, sector.oodleStop0);
			Platform_Swap2(gr2->data + ofs + sector.oodleStop0, sector.oodleStop1 - sector.oodleStop0);
		}

		gr2->sectorOffsets[i] = ofs;
		ofs += sector.decompressLen;
	}

	/* read sector data to apply marshalling and fixup */
	for (i = 0; i < gr2->fileInfo.sectorCount; i++)
	{
		TSector sector = gr2->sectors[i];
		uint32_t k;
		size_t pos;

		for (k = 0; k < sector.marshallSize; k++)
		{
			pos = sector.marshallOffset + (k * sizeof(TFixUpData));

			if (pos > len)
			{
				dbg_printf("out of bounds");
				return false;
			}

			TMarshallData* md = (TMarshallData*)(data + pos);

			if (gr2->mismatchEndianness)
			{
				Platform_Swap1((uint8_t*)md, sizeof(TMarshallData));
				Gr2_ApplyMarshall(gr2, i, md, data);
			}
		}

		for (k = 0; k < sector.fixupSize; k++)
		{
			pos = sector.fixupOffset + (k * sizeof(TFixUpData));

			if (pos > len)
			{
				dbg_printf("out of bounds");
				return false;
			}

			TFixUpData* fd = (TFixUpData*)(data + pos);

			if (gr2->mismatchEndianness)
				Platform_Swap1((uint8_t*)fd, sizeof(TFixUpData));

			Gr2_ApplyFixUp(gr2, i, fd);
		}
	}

	/* file parsing completed! begin node loading */
	return Element_Parse(gr2->data + gr2->sectorOffsets[gr2->fileInfo.type.sector] + gr2->fileInfo.type.position, gr2->data + gr2->sectorOffsets[gr2->fileInfo.root.sector] + gr2->fileInfo.root.position, magicFlags & MAGIC_FLAG_64BIT, &gr2->elements, &gr2->root);
}

OG_DLLAPI bool Gr2_Init(TGr2* gr2)
{
	memset(gr2, 0, sizeof(TGr2));
	
	if (!Element_New(TYPEID_INLINE, "Root", &gr2->root))
		return false;

	return DArray_Init(&gr2->elements, sizeof(TElementInfo), 15);
}

OG_DLLAPI void Gr2_Free(TGr2* gr2)
{
	Element_Free(&gr2->root);

	for (size_t i = 0; i < gr2->elements.count; i++)
	{
		Element_Free((TElementInfo*)DArray_Get(&gr2->elements, i));
	}

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

bool OG_DLLAPI Gr2_AddElement(TGr2* gr2, TElementInfo* elem, bool addToRoot, size_t* posOut)
{
	size_t pos;

	if (!DArray_Add(&gr2->elements, elem))
		return false;

	pos = gr2->elements.count - 1;

	if (addToRoot)
		Element_AddToChild(&gr2->root, pos);

	if (posOut)
		*posOut = pos;

	return true;
}

bool OG_DLLAPI Gr2_Compose(TGr2* gr2)
{
	size_t completeSize;

	for (size_t i = 0; i < gr2->elements.count; i++)
	{

	}
}