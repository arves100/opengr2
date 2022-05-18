/*!
	Project: libopengrn
	File: platform.c
	Platform utility

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "platform.h"

/*!
	Gets the pointer size of the platform
	@return the pointer size
*/
int Platform_GetPointerSize()
{
    return sizeof(void*);
}

/*!
	Checks if the platform is big endian
	@return true if the platform is big endian
*/
bool Platform_IsBigEndian()
{
    uint16_t t = 1;
    const unsigned char* c = (unsigned char*)(&t);

    /*
		*c == 1 -> little endian
		*c != 1 -> big endian
	*/

    return *c != 1;
}

/*!
	Swap bytes for endianness mismatch (type1)
	@param data the data to swap
	@param len the length of the data
*/
void Platform_Swap1(uint8_t* data, size_t len)
{
	if (len < 4)
		return;

	for (size_t i = 0; i < len / 4; i++)
	{
		uint8_t d3 = data[i + 3], d2 = data[i + 2];

		data[i + 3] = data[i];
		data[i + 2] = data[i + 1];
		data[i + 1] = d2;
		data[i] = d3;
	}
}

/*!
	Swap bytes for endianness mismatch (type2)
	@param data the data to swap
	@param len the length of the data
*/
void Platform_Swap2(uint8_t* data, size_t len)
{
	if (len < 4)
		return;

	for (size_t i = 0; i < len / 4; i++)
	{
		uint8_t d0 = data[i], d2 = data[i + 2];

		data[i] = data[i + 1];
		data[i + 1] = d0;
		data[i + 2] = data[i + 3];
		data[i + 3] = d2;
	}
}
