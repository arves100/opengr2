/*!
	Project: libopengrn
	File: platform.h
	Platform utility

	Copyright (C) 2021 Arves100
*/
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "dllapi.h"


/*!
	Gets the pointer size of the platform
	@return the pointer size
*/
inline int Platform_GetPointerSize()
{
	return sizeof(void*);
}

/*!
	Checks if the platform is big endian
	@return true if the platform is big endian
*/
inline bool Platform_IsBigEndian()
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
extern OG_DLLAPI void Platform_Swap1(uint8_t* data, size_t len);

/*!
	Swap bytes for endianness mismatch (type2)
	@param data the data to swap
	@param len the length of the data
*/
extern OG_DLLAPI void Platform_Swap2(uint8_t* data, size_t len);
