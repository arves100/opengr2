#pragma once

#include <stdint.h>

#include "dllapi.h"

extern OG_DLLAPI uint32_t encode_ptr(const void *ptr);
extern OG_DLLAPI void* decode_ptr(uint32_t ptr);