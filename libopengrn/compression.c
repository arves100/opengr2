/*!
	Project: libopengrn
	File: compression.h
	Compression/Decompression API

	Copyright (C) 2021 Arves100
*/
#include "compression.h"

/*!
	Gets the extra bytes that needs to be allocated for the specific compression
	@param nType the compression type
	@return the extra bytes to allocate
*/
int OG_DLLAPI Compression_GetExtraLen(uint32_t nType)
{
	return (nType == COMPRESSION_TYPE_OODLE0 || nType == COMPRESSION_TYPE_OODLE1) ? 4 : 0;
}
