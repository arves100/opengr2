/*!
    Project: libopengrn
    File: oodle1.c
    Oodle1 compression/decompression functions

    Oodle1 compression code is derived from https://github.com/Arbos/nwn2mdk/blob/master/nwn2mdk-lib/gr2_decompress.cpp

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "oodle1.h"
#include <malloc.h>
#include <memory.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#define min(x, y) ((x) > (y)) ? (y) : (x)
#define max(x, y) ((x) > (y)) ? (x) : (y)

void Decoder_Init(TDecoder *decoder, uint8_t* stream) {
    decoder->numer = stream[0] >> 1;
    decoder->denom = 0x80;
    decoder->stream = stream;
}

uint16_t Decode(TDecoder *decoder, uint16_t max) {
    for(; decoder->denom <= 0x800000; decoder->denom <<= 8) {
        decoder->numer <<= 8;
        decoder->numer |= (decoder->stream[0] << 7) & 0x80;
        decoder->numer |= (decoder->stream[1] >> 1) & 0x7f;
        decoder->stream++;
    }

    decoder->next_denom = decoder->denom / max;
    return min(decoder->numer / decoder->next_denom, max - 1);
}

uint16_t Commit(TDecoder *decoder, uint16_t max, uint16_t val, uint16_t err) {
    decoder->numer -= decoder->next_denom * val;

    if(val + err < max) {
        decoder->denom = decoder->next_denom * err;
    } else {
        decoder->denom -= decoder->next_denom * val;
    }

    return val;
}

uint16_t Decode_Commit(TDecoder *decoder, uint16_t max) {
    return Commit(decoder, max, Decode(decoder, max), 1);
}

void WeighWindow_Init(TWeighWindow *weighWindow, uint32_t maxValue, uint16_t countCap) {
    weighWindow->weight_total = 4;
    weighWindow->count_cap = countCap + 1;

    weighWindow->rangesLength = 2;
    weighWindow->ranges = malloc(sizeof(uint16_t) * weighWindow->rangesLength);
    weighWindow->ranges[0] = 0;
    weighWindow->ranges[1] = 0x4000;

    weighWindow->weightsLength = 1;
    weighWindow->weights = malloc(sizeof(uint16_t) * weighWindow->weightsLength);
    weighWindow->weights[0] = 4;

    weighWindow->valuesLength = 1;
    weighWindow->values = malloc(sizeof(uint16_t) * weighWindow->valuesLength);
    weighWindow->values[0] = 0;

    weighWindow->thresh_increase = 4;
    weighWindow->thresh_range_rebuild = 8;
    weighWindow->thresh_weight_rebuild = max(256u, min(32 * maxValue, 15160u));

    if (maxValue > 64)
        weighWindow->thresh_increase_cap = min(2 * maxValue, weighWindow->thresh_weight_rebuild / 2 - 32u);
    else
        weighWindow->thresh_increase_cap = 128;
}

void WeightWindow_Rebuild_Ranges(TWeighWindow *weighWindow) {
    //printf("REBUILD RANGES: %i ", weighWindow->weightsLength);
    if(weighWindow->rangesLength != weighWindow->weightsLength + 1) {
        uint16_t *oldBuffer = weighWindow->ranges;
        //size_t oldLength = weighWindow->rangesLength;
        weighWindow->ranges = malloc(sizeof(uint16_t) * (weighWindow->weightsLength + 1));
        weighWindow->rangesLength = weighWindow->weightsLength + 1;

        //memcpy_s(weighWindow->ranges, weighWindow->rangesLength * sizeof(uint16_t), oldBuffer, oldLength * sizeof(uint16_t));
        free(oldBuffer);
    }

    uint16_t range_weight = 8 * 0x4000 / weighWindow->weight_total;
    //printf("%i ", range_weight);
    uint16_t range_start = 0;
    for(size_t i = 0; i < weighWindow->weightsLength; i++) {
        weighWindow->ranges[i] = range_start;
        //printf("%i ", range_start);
        range_start += (weighWindow->weights[i] * range_weight) / 8;
    }
    weighWindow->ranges[weighWindow->rangesLength - 1] = 0x4000;

    if (weighWindow->thresh_increase > weighWindow->thresh_increase_cap / 2) {
        weighWindow->thresh_range_rebuild = weighWindow->weight_total + weighWindow->thresh_increase_cap;
    }
    else {
        weighWindow->thresh_increase *= 2;
        weighWindow->thresh_range_rebuild = weighWindow->weight_total + weighWindow->thresh_increase;
    }
    //printf("%i %i\n", weighWindow->thresh_range_rebuild, weighWindow->thresh_range_rebuild);
}

size_t max_element(const uint16_t* arr, size_t length, size_t offset) {
    uint16_t max = 0;
    size_t index = offset;

    for(size_t i = offset; i < length; i++) {
        if(arr[i] > max) {
            max = arr[i];
            index = i;
        }
    }

    return index;
}

void WeightWindow_Rebuild_Weights(TWeighWindow *weighWindow) {
    //printf("REBUILD WEIGHTS: ");

    uint16_t weight_total = 0;
    for(size_t i = 0; i < weighWindow->weightsLength; i++) {
        weighWindow->weights[i] /= 2;
        //printf("%i ", weighWindow->weights[i]);

        weight_total += weighWindow->weights[i];
    }

    weighWindow->weight_total = weight_total;
    //printf("%i ", weighWindow->weight_total);

    for (uint32_t i = 1; i < weighWindow->weightsLength; i++) {
        while (i < weighWindow->weightsLength && weighWindow->weights[i] == 0) {
            weighWindow->weights[i] = weighWindow->weights[weighWindow->weightsLength - 1];
            weighWindow->values[i] = weighWindow->values[weighWindow->valuesLength - 1];

            weighWindow->weightsLength--;
            weighWindow->valuesLength--;
            //printf("!");
        }
    }

    size_t it = max_element(weighWindow->weights, weighWindow->weightsLength, 1);
    //printf(" %i ", weighWindow->weights[it]);
    if (it < weighWindow->weightsLength) {
        size_t i = it;
        //printf("%i ", i);

        uint16_t tmp = weighWindow->weights[i];
        weighWindow->weights[i] = weighWindow->weights[weighWindow->weightsLength - 1];
        weighWindow->weights[weighWindow->weightsLength - 1] = tmp;

        tmp = weighWindow->values[i];
        weighWindow->values[i] = weighWindow->values[weighWindow->valuesLength - 1];
        weighWindow->values[weighWindow->valuesLength - 1] = tmp;
    }

    if ((weighWindow->weightsLength < weighWindow->count_cap) && (weighWindow->weights[0] == 0)) {
        weighWindow->weights[0] = 1;
        weighWindow->weight_total++;
        //printf("$");
    }
    //printf("\n");
}

IndexValuePair WeightWindow_Try_Decode(TWeighWindow *weighWindow, TDecoder *decoder) {
    //printf("%i %i %i %i\n", weighWindow->weight_total, weighWindow->thresh_range_rebuild, weighWindow->thresh_range_rebuild, weighWindow->thresh_weight_rebuild);
    if (weighWindow->weight_total >= weighWindow->thresh_range_rebuild) {
        if (weighWindow->thresh_range_rebuild >= weighWindow->thresh_weight_rebuild)
            WeightWindow_Rebuild_Weights(weighWindow);
        WeightWindow_Rebuild_Ranges(weighWindow);
    }

    uint16_t value = Decode(decoder, 0x4000);
    size_t rangeit = weighWindow->rangesLength - 1;// std::upper_bound(std::begin(this->ranges), std::end(this->ranges), value) - 1;
    for(size_t i = 0; i < weighWindow->rangesLength; i++) {
        if(weighWindow->ranges[i] > value) {
            rangeit = i;
            break;
        }
    }
    if(rangeit == 0) {
#ifdef _MSC_VER
        _CrtDbgBreak();
#else
        asm("int $3");
#endif
        rangeit = 0;
    }
    rangeit--;
    //printf("%i %i %i ", value, weighWindow->ranges[rangeit], weighWindow->ranges[rangeit + 1]);

    Commit(decoder, 0x4000, weighWindow->ranges[rangeit], weighWindow->ranges[rangeit + 1] - weighWindow->ranges[rangeit]);

    size_t index = rangeit;
    //printf("%i ", index);
    weighWindow->weights[index]++;
    weighWindow->weight_total++;
    //printf("%i %i ", weighWindow->weights[index], weighWindow->weight_total);

    if (index > 0) {
        //printf("\n");
        IndexValuePair ret;
        ret.index = 0xFFFF;
        ret.value = weighWindow->values[index];
        return ret;
    }

    if ((weighWindow->weightsLength >= weighWindow->rangesLength)
        && (Decode_Commit(decoder, 2) == 1)) {
        //printf("*%i %i ", weighWindow->rangesLength, weighWindow->weightsLength);
        size_t index = weighWindow->rangesLength + Decode_Commit(decoder, weighWindow->weightsLength - weighWindow->rangesLength + 1) - 1;
        //printf("%i ", index);

        weighWindow->weights[index] += 2;
        weighWindow->weight_total += 2;
        //printf("%i %i!\n", weighWindow->weights[index], weighWindow->weight_total);

        IndexValuePair ret;
        ret.index = 0xFFFF;
        ret.value = weighWindow->values[index];
        return ret;
    }

    size_t oldLength = weighWindow->valuesLength;
    uint16_t* oldBuffer = weighWindow->values;
    weighWindow->valuesLength++;
    weighWindow->values = malloc(weighWindow->valuesLength * sizeof(uint16_t));
    memcpy_s(weighWindow->values, weighWindow->valuesLength * sizeof(uint16_t), oldBuffer, oldLength * sizeof(uint16_t));
    free(oldBuffer);
    weighWindow->values[weighWindow->valuesLength - 1] = 0;

    oldLength = weighWindow->weightsLength;
    oldBuffer = weighWindow->weights;
    weighWindow->weightsLength++;
    weighWindow->weights = malloc(weighWindow->weightsLength * sizeof(uint16_t));
    memcpy_s(weighWindow->weights, weighWindow->weightsLength * sizeof(uint16_t), oldBuffer, oldLength * sizeof(uint16_t));
    free(oldBuffer);
    weighWindow->weights[weighWindow->weightsLength - 1] = 2;

    weighWindow->weight_total += 2;
    //printf("%i ", weighWindow->weight_total);

    if (weighWindow->weightsLength == weighWindow->count_cap) {
        weighWindow->weight_total -= weighWindow->weights[0];
        weighWindow->weights[0] = 0;

        //printf("$%i %i ", weighWindow->weight_total, weighWindow->weights[0]);
    }

    IndexValuePair ret;
    ret.index = weighWindow->valuesLength - 1;
    ret.value = 0;
    //printf("\n");
    return ret;
}

void Dictionary_Init(TDictionary *dictionary, TParameter *parameter) {
    dictionary->decoded_size = 0;
    dictionary->backref_size = 0;

    dictionary->decoded_value_max = parameter->decoded_value_max;
    dictionary->backref_value_max = parameter->backref_value_max;
    dictionary->lowbit_value_max = min(dictionary->backref_value_max + 1, 4);
    dictionary->midbit_value_max = min(dictionary->backref_value_max / 4 + 1, 256);
    dictionary->highbit_value_max = dictionary->backref_value_max / 1024 + 1;

    WeighWindow_Init(&dictionary->lowbit_window, dictionary->lowbit_value_max - 1, dictionary->lowbit_value_max);
    WeighWindow_Init(&dictionary->highbit_window, dictionary->highbit_value_max - 1, parameter->highbit_count + 1);

    dictionary->midbit_windows = malloc(sizeof(TWeighWindow) * dictionary->highbit_value_max);
    for (size_t i = 0; i < dictionary->highbit_value_max; ++i) {
        WeighWindow_Init(&dictionary->midbit_windows[i], dictionary->midbit_value_max - 1, dictionary->midbit_value_max);
    }

    dictionary->decoded_windows = malloc(sizeof(TWeighWindow) * 4);
    for (size_t i = 0; i < 4; ++i) {
        WeighWindow_Init(&dictionary->decoded_windows[i], dictionary->decoded_value_max - 1, parameter->decoded_count);
    }

    size_t index = 0;
    dictionary->size_windows = malloc(sizeof(TWeighWindow) * (4 * 16 + 1));
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 16; ++j) {
            WeighWindow_Init(&dictionary->size_windows[index++], 64, parameter->sizes_count[3 - i]);
        }
    }
    WeighWindow_Init(&dictionary->size_windows[index++], 64, parameter->sizes_count[0]);
}

uint32_t Dictionary_Decompress_Block(TDictionary *dictionary, TDecoder *decoder, uint8_t *decompressedData) {
    //printf("%i %i %i %i\n", dictionary->backref_size, dictionary->backref_value_max, dictionary->decoded_size, dictionary->lowbit_value_max);

    IndexValuePair d1 = WeightWindow_Try_Decode(&dictionary->size_windows[dictionary->backref_size], decoder);

    if (d1.index != 0xFFFF) {
        d1.value = (dictionary->size_windows[dictionary->backref_size].values[d1.index] = Decode_Commit(decoder, 65));
    }
    //printf("d1 %i\n", d1.value);
    dictionary->backref_size = d1.value;

    if (dictionary->backref_size > 0) {
        static uint32_t const sizes[] = { 128u, 192u, 256u, 512u };

        uint32_t backref_size = dictionary->backref_size < 61u ? dictionary->backref_size + 1 : sizes[dictionary->backref_size - 61u];
        uint32_t backref_range = min(dictionary->backref_value_max, dictionary->decoded_size);

        IndexValuePair d3 = WeightWindow_Try_Decode(&dictionary->lowbit_window, decoder);
        if (d3.index != 0xFFFF) {
            d3.value = (dictionary->lowbit_window.values[d3.index] = Decode_Commit(decoder, dictionary->lowbit_value_max));
        }

        IndexValuePair d4 = WeightWindow_Try_Decode(&dictionary->highbit_window, decoder);
        if (d4.index != 0xFFFF) {
            d4.value = (dictionary->highbit_window.values[d4.index] = Decode_Commit(decoder, backref_range / 1024u + 1));
        }

        IndexValuePair d5 = WeightWindow_Try_Decode(&dictionary->midbit_windows[d4.value], decoder);
        if (d5.index != 0xFFFF) {
            d5.value = (dictionary->midbit_windows[d4.value].values[d5.index] = Decode_Commit(decoder, min(backref_range / 4 + 1, 256)));
        }

        uint32_t backref_offset = (d4.value << 10) + (d5.value << 2) + d3.value + 1u;

        dictionary->decoded_size += backref_size;

        size_t repeat = backref_size / backref_offset;
        size_t remain = backref_size % backref_offset;
        for (size_t i = 0; i < repeat; ++i) {
            memcpy(decompressedData + i * backref_offset, decompressedData - backref_offset, backref_offset);
        }
        memcpy(decompressedData + repeat * backref_offset, decompressedData - backref_offset, remain);

        //printf("d3 %i\n", d3.value);
        //printf("d4 %i\n", d4.value);
        //printf("d5 %i\n\n", d5.value);
        return backref_size;
    }
    else {
        uintptr_t i = (uintptr_t)decompressedData % 4;
        IndexValuePair d2 = WeightWindow_Try_Decode(&dictionary->decoded_windows[i], decoder);
        if (d2.index != 0xFFFF) {
            d2.value = (dictionary->decoded_windows[i].values[d2.index] = Decode_Commit(decoder, dictionary->decoded_value_max));
        }

        decompressedData[0] = d2.value & 0xff;
        dictionary->decoded_size++;

        //printf("d2 %i\n\n", d2.value);
        return 1;
    }
}