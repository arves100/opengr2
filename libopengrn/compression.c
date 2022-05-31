/*!
	Project: libopengrn
	File: compression.h
	Compression/Decompression API

    Oodle1 compression code is derived from https://github.com/Arbos/nwn2mdk/blob/master/nwn2mdk-lib/gr2_decompress.cpp

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "compression.h"
#include "platform.h"
#include "oodle1.h"

#include <memory.h>

/*!
	Gets the extra bytes that needs to be allocated for the specific compression
	@param nType the compression type
	@return the extra bytes to allocate
*/
int Compression_GetExtraLen(uint32_t nType)
{
	return (nType == COMPRESSION_TYPE_OODLE0 || nType == COMPRESSION_TYPE_OODLE1) ? 4 : 0;
}

/*!
    Decompresses data with algorithm Oodle-1
    @param compressedData the compressed data to decompress
    @param compressedLength length of the compressed data
    @param decompressedData A buffer which will store the decompressed data
    @param decompressedLength length of the decompressed data
    @param oodleStop1 first stop byte of oodle
    @param oodleStop2 second stop byte of oodle
    @return true if the decompression succeeded, otherwise false
*/
bool Compression_UnOodle1(uint8_t* compressedData,
                          uint32_t compressedLength,
                          uint8_t* decompressedData,
                          uint32_t decompressedLength,
                          uint32_t oodleStop1,
                          uint32_t oodleStop2,
                          bool endianessMismatch) {
    if(compressedLength == 0) {
        return true;
    }

    TParameter parameters[3];

    memset(parameters, 0, sizeof(parameters));
    memcpy(parameters, compressedData, sizeof(parameters));

    if (endianessMismatch)
        Platform_Swap1((uint8_t*)parameters, sizeof(parameters));

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

        Dictionary_Free(&dict);
    }

    return true;
}
