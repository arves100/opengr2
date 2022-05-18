/*!
	Project: libopengrn
	File: virtual_ptr.h
	Virtual pointer utilities

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <stdint.h>

#include "dllapi.h"
#include "gr2.h"

extern uint32_t encode_ptr(TDArray* array, const void *ptr);
extern void* decode_ptr(TDArray* array, uint32_t ptr);
