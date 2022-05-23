/*!
	Project: libopengrn
	File: compression.h
	Compression/Decompression API

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#pragma once

#include "dllapi.h"
#include <stdbool.h>
#include <stdint.h>

/*!
	@enum ECompressionTypes
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
extern int Compression_GetExtraLen(uint32_t nType);

/*!
	Decompresses data with algorithm Oodle-1
	@param compressedData the compressed data to decompress
	@param compressedLength length of the compressed data
	@param decompressedData A buffer which will store the decompressed data
	@param decompressedLength length of the decompressed data
	@param oodleStop1 first stop byte of oodle
	@param oodleStop2 second stop byte of oodle
	@param endianessMismatch if the file has a different endianess
	@return true if the decompression succeeded, otherwise false
*/
extern bool Compression_UnOodle1(uint8_t* compressedData,
                                           uint32_t compressedLength,
                                           uint8_t* decompressedData,
                                           uint32_t decompressedLength,
                                           uint32_t oodleStop1,
                                           uint32_t oodleStop2,
	                                       bool endianessMismatch);
