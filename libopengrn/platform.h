/*!
	Project: libopengrn
	File: platform.h
	Platform utility

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <stdbool.h>
#include <stdint.h>

/*!
	Gets the pointer size of the platform
	@return the pointer size
*/
extern inline int Platform_GetPointerSize();

/*!
	Checks if the platform is big endian
	@return true if the platform is big endian
*/
extern inline bool Platform_IsBigEndian();

/*!
	Swap bytes for endianness mismatch (type1)
	@param data the data to swap
	@param len the length of the data
*/
extern void Platform_Swap1(uint8_t* data, size_t len);

/*!
	Swap bytes for endianness mismatch (type2)
	@param data the data to swap
	@param len the length of the data
*/
extern void Platform_Swap2(uint8_t* data, size_t len);
