// Oodle1 compression code is derived from https://github.com/Arbos/nwn2mdk/blob/master/nwn2mdk-lib/gr2_decompress.cpp

/*!
	Project: libopengrn
	File: compression.h
	Compression/Decompression API

	Copyright (C) 2021 Arves100
*/
#include "compression.h"
#include <memory.h>

#include "oodle1.h"

/*!
	Gets the extra bytes that needs to be allocated for the specific compression
	@param nType the compression type
	@return the extra bytes to allocate
*/
int OG_DLLAPI Compression_GetExtraLen(uint32_t nType)
{
	return (nType == COMPRESSION_TYPE_OODLE0 || nType == COMPRESSION_TYPE_OODLE1) ? 4 : 0;
}

bool OG_DLLAPI Compression_UnOodle1(uint8_t* compressedData,
                          uint32_t compressedLength,
                          uint8_t* decompressedData,
                          uint32_t decompressedLength,
                          uint32_t oodleStop1,
                          uint32_t oodleStop2) {
    if(compressedLength == 0) {
        return true;
    }

    TParameter parameters[3] = {};
    memcpy(parameters, compressedData, sizeof(parameters));

    TDecoder decoder;
    Decoder_Init(&decoder, compressedData + sizeof(parameters));
    uint32_t steps[] = { oodleStop1, oodleStop2, decompressedLength };
    uint8_t* ptr = decompressedData;

    for(uint32_t i = 0; i < 3; i++) {
        TDictionary dict;
        Dictionary_Init(&dict, &parameters[i]);

        while(ptr < decompressedData + steps[i]) {
            ptr += Dictionary_Decompress_Block(&dict, &decoder, ptr);
        }
    }

    return true;
}