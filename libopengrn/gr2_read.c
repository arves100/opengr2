/*!
	Project: libopengrn
	File: gr2_read.c
	Low Level Gr2 read api

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "gr2.h"
#include "debug.h"
#include "compression.h"
#include "magic.h"
#include "virtual_ptr.h"

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
		Platform_Swap1((uint8_t*)&gr2->fileInfo, sizeof(gr2->fileInfo));

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
	void* dst = gr2->data + gr2->sectorOffsets[fd->dstSector] + fd->dstOffset;
	void* src = gr2->data + gr2->sectorOffsets[srcSector] + fd->srcOffset;
	uint32_t dstPtr = encode_ptr(&gr2->virtual_ptr, dst);

	memcpy_s(src, 4, &dstPtr, 4); /* 4 -> Pointer size on 32bit */
}

/*!
	Applies marshalling fix for endianness mismatch situations
	@param gr2 The gr2 file to fix
	@param md marshalling information
*/
static void Gr2_ApplyMarshall(TGr2* gr2, uint32_t srcSector, TMarshallData* md, const uint8_t* data)
{
	int dataSize, flag, i = 0;

	// TODO: verify if this code is ok

	for (; i < md->count; i++)
	{
		uint8_t* swapData = (uint8_t*)gr2->data[gr2->sectorOffsets[srcSector]] + md->srcOffset;
		// TODO: t_Type from element nodes...
	}
	/*
			Marshal op:
			1. Get Marshal data structure
			you will have the current sector you are processing + an offset to start marshalling data
			you will have a target sector + target offset

			The target sector define the type to actually swap (it's a t_Type* structure)
			if the type == 1, simply process it's childrens

			The swap is done by 4 bytes each:
			t_Type* curr = (t_Type*)&marshallData->targetSector[marshallData->targetOffset];
			BYTE* swapData = (BYTE*)&currentSector[marshallData->sourceOffset];

			int i = 0, flg, ln = 0;

			jmp_inline:
			for (i= 0; i < marshallData->size; i++)
			{
				while (curr->type != TYPE_NONE)
				{
					if (curr->type == TYPE_INLINE)
						goto jmp_inline;
					else
					{
						ln = GetNodeDataSize(curr);
						flg = GetProcessFlag(curr->type) - 2;
						if (flg == 2)
							SwapBytes(curr, lm);
						else if (flag < 0)
							SwapBytes2(curr, lm);
					}
					curr = curr->next->type;
					dst += ln;
				}
			}
	*/
}

OG_DLLAPI bool Gr2_Load(const uint8_t* data, size_t len, TGr2* gr2)
{
	uint32_t i;
	size_t ofs = 0;
	uint8_t magicFlags;
	uint64_t rootOffset = 0;

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

	if (magicFlags & MAGIC_FLAG_64BIT)
		gr2->bitsSize = 64;
	else
		gr2->bitsSize = 32;

	gr2->mismatchEndianness = Platform_IsBigEndian() != (magicFlags & MAGIC_FLAG_BIGENDIAN);

	gr2->header = *(THeader*)data;

	if (gr2->mismatchEndianness)
		Platform_Swap1((uint8_t*)&gr2->header.sizeWithSectors, 4);

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
			uint8_t* pComp = (uint8_t*)malloc(sector.compressedLen + extraLen), * pDecomp;
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

			pDecomp = (uint8_t*)malloc(sector.decompressLen);

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
				free(pDecomp);
				free(pComp);

				dbg_printf("invalid/unsupported compression %d", sector.compressType);
				return false;
			}

			free(pComp);

			if (!success)
			{
				dbg_printf("decompression of %d fail", sector.compressType);
				free(pDecomp);
				return false;
			}
			else {
				memcpy_s(gr2->data + ofs, gr2->dataSize - ofs, pDecomp, sector.decompressLen);

				if (gr2->mismatchEndianness)
					Platform_Swap1(gr2->data + ofs, sector.decompressLen); /* should be done on compressed data as well */
			}

			free(pDecomp);
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
	return Element_Parse(&gr2->virtual_ptr, gr2->data + gr2->sectorOffsets[gr2->fileInfo.type.sector] + gr2->fileInfo.type.position, gr2->data + gr2->sectorOffsets[gr2->fileInfo.root.sector] + gr2->fileInfo.root.position, magicFlags & MAGIC_FLAG_64BIT, &gr2->elements, gr2->root, &rootOffset);
}
