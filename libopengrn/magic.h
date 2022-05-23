/*!
	Project: libopengrn
	File: magic.h
	Magic utilities

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#pragma once

#include "dllapi.h"

#include <stdint.h>
#include <stdbool.h>

/*!
	Current known magic flags
*/
enum EMagicFlags
{
	MAGIC_FLAG_NONE = 0,
	MAGIC_FLAG_BIGENDIAN = 1 << 0,
	MAGIC_FLAG_64BIT = 1 << 1, /* Pointer sizes is 64-bit not 32-bit */
	MAGIC_FLAG_EXTRA16 = 1 << 2, /* File Format 7 (Extra 16 bytes in FileInfo) */
};

/*!
	Informations about gr2 magic
*/
typedef struct SMagicInfo
{
	uint8_t flags; /* flags of the magic */
	uint32_t magic[4]; /* magic value */
} TMagicInfo;

/*!
	Global holder of magic info
*/
extern OG_DLLAPI const TMagicInfo MAGIC_DATA[];

/*!
	Gets the information of a magic
	@param magic the magic to test
	@flags a pointer that returns the magic info
	@return true if the magic exists, otherwise false
*/
extern OG_DLLAPI bool Magic_GetFlags(const uint32_t* magic, uint8_t* flags);

extern OG_DLLAPI void Magic_Set(uint32_t* magic, uint8_t flags);
