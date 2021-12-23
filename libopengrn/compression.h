/*!
	Project: libopengrn
	File: compression.h
	Compression/Decompression API

	Copyright (C) 2021 Arves100
*/
#pragma once

#include <stdint.h>
#include "dllapi.h"

/*!
	Currently known compression types
*/
enum ECompressionTypes
{
	COMPRESSION_TYPE_NONE, /* No compression */
	COMPRESSION_TYPE_OODLE0,
	COMPRESSION_TYPE_OODLE1,
	COMPRESSION_TYPE_BITKNIT1,
	COMPRESSION_TYPE_BITKNIT2,
};


/*!
	Gets the extra bytes that needs to be allocated for the specific compression
	@param nType the compression type
	@return the extra bytes to allocate
*/
extern OG_DLLAPI int Compression_GetExtraLen(uint32_t nType);
