/*!
	Project: libopengrn
	File: magic.c
	Magic utilities

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "magic.h"

#include <string.h>

/*!
	Global holder of magic info
*/
OG_DLLAPI const TMagicInfo MAGIC_DATA[3] = {
	{ MAGIC_FLAG_NONE, {3400558520, 263286264, 2123133572, 503322974} }, /* Little Endian 32-bit File Format 6 */
	{ MAGIC_FLAG_BIGENDIAN, {3093803210, 4167938319, 2222099582, 1578696734} }, /* Big Endian 32-bit File Format 6 */

	{ MAGIC_FLAG_EXTRA16, {3228360233, 726901946, 2780296485, 4007814902} } /* Little Endian 32-bit File Format 7 (Granny 2.9) */
	/* LE 64-bit FF 7 */
	/* BE 32-bit FF 7 */
	/* BE 64-bit FF 7 */
};

/*!
	Gets the information of a magic
	@param magic the magic to test
	@flags a pointer that returns the magic info
	@return true if the magic exists, otherwise false
*/
bool OG_DLLAPI Magic_GetFlags(const uint32_t* magic, uint8_t* flags)
{
	for (int i = 0; i < (sizeof(MAGIC_DATA) / sizeof(TMagicInfo)); i++)
	{
		if (memcmp(magic, MAGIC_DATA[i].magic, 16) == 0)
		{
			*flags = MAGIC_DATA[i].flags;
			return true;
		}
	}

	return false;
}

void OG_DLLAPI Magic_Set(uint32_t* magic, uint8_t flags)
{
	for (int i = 0; i < (sizeof(MAGIC_DATA) / sizeof(TMagicInfo)); i++)
	{
		if (flags == MAGIC_DATA[i].flags)
		{
			memcpy(magic, MAGIC_DATA[i].magic, 16);
		}
	}
}
