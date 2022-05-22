/*!
    Project: libopengrn
    File: oodle1.c
    Oodle1 compression/decompression functions

    Oodle1 compression code is derived from https://github.com/Arbos/nwn2mdk/blob/master/nwn2mdk-lib/gr2_decompress.cpp

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#pragma once

#include "dllapi.h"

#include <string.h>
#include <stdint.h>

typedef struct {
    unsigned decoded_value_max : 9;
    unsigned backref_value_max : 23;
    unsigned decoded_count : 9;
    unsigned padding : 10;
    unsigned highbit_count : 13;
    uint8_t sizes_count[4];
} TParameter;

typedef struct {
    uint32_t numer;
    uint32_t denom;
    uint32_t next_denom;
    uint8_t* stream;
} TDecoder;

typedef struct {
    uint16_t count_cap;

    uint16_t* ranges;
    size_t rangesLength;

    uint16_t* values;
    size_t valuesLength;

    uint16_t* weights;
    size_t weightsLength;

    uint16_t weight_total;

    uint16_t thresh_increase;
    uint16_t thresh_increase_cap;
    uint16_t thresh_range_rebuild;
    uint16_t thresh_weight_rebuild;
} TWeighWindow;

typedef struct {
    uint32_t decoded_size;
    uint32_t backref_size;

    uint32_t decoded_value_max;
    uint32_t backref_value_max;
    uint32_t lowbit_value_max;
    uint32_t midbit_value_max;
    uint32_t highbit_value_max;

    TWeighWindow lowbit_window;
    TWeighWindow highbit_window;
    TWeighWindow* midbit_windows;

    TWeighWindow* decoded_windows;
    TWeighWindow* size_windows;
} TDictionary;

typedef struct {
    size_t index;
    uint16_t value;
} IndexValuePair;

extern void Decoder_Init(TDecoder *decoder, uint8_t* stream);
extern uint16_t Decode(TDecoder *decoder, uint16_t max);
extern uint16_t Commit(TDecoder *decoder, uint16_t max, uint16_t val, uint16_t err);
extern uint16_t Decode_Commit(TDecoder *decoder, uint16_t max);

extern void WeighWindow_Init(TWeighWindow *weighWindow, uint32_t maxValue, uint16_t countCap);
extern void WeighWindow_Free(TWeighWindow *weighWindow);
extern void WeightWindow_Rebuild_Weights(TWeighWindow *weighWindow);
extern void WeightWindow_Rebuild_Ranges(TWeighWindow *weighWindow);
extern IndexValuePair WeightWindow_Try_Decode(TWeighWindow *weighWindow, TDecoder *decoder);

extern void Dictionary_Init(TDictionary *dictionary, TParameter *parameter);
extern uint32_t Dictionary_Decompress_Block(TDictionary *dictionary, TDecoder *decoder, uint8_t *decompressedData);
extern void Dictionary_Free(TDictionary *dictionary);
