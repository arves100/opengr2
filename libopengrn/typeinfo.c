/*!
	Project: libopengrn
	File: typeinfo.c
	Internal node type utility and parsing

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "typeinfo.h"
#include "virtual_ptr.h"

#include <string.h>

OG_DLLAPI TTypeInfo ELEMENT_TYPE_INFO[23] =
{
	{ 0, 0, 0 }, // NULL
	{ 0, 0, 0 }, // NULL
	{ 4, 8, 0 }, // void*
	{ 8, 12, 4 }, //void**
	{ 8, 12, 4 }, // void***
	{ 8, 16, 0 }, // void*
	{ 0, 0, 0, }, // __REMOVED
	{ 12, 20, 4 }, // void*
	{ 4, 8, 4, }, // char*
	{ 68, 68, 4, }, // gr2_transform
	{ 4, 4, 4, }, // float
	{ 1, 1, 1, }, // int8_t
	{ 1, 1, 1, }, // uint8_t
	{ 1, 1, 1, }, // int8_t
	{ 1, 1, 1, }, // uint8_t
	{ 2, 2, 2, }, // int16_t
	{ 2, 2, 2, }, // uint16_t
	{ 2, 2, 2, }, // int16_t
	{ 2, 2, 2, }, // uint16_t
	{ 4, 4, 4, }, // int32_t
	{ 4, 4, 4, }, // uint32_t
	{ 2, 2, 2, }, // float/2
	{ 4, 8, 0, }, // void*
};

bool TypeInfo_Parse(const uint8_t* data, TNodeTypeInfo* info, bool is64, uint64_t* offset)
{
	TNodeTypeInfo ni;

	ni.type = *(uint32_t*)(data + *offset);

	if (ni.type == 0 || ni.type > TYPEID_MAX)
		return false;

	*offset += 4;

	if (is64)
	{
		ni.nameOffset = *(uint64_t*)(data + *offset);
		*offset += 8;
		ni.childrenOffset = *(uint64_t*)(data + *offset);
		*offset += 16;
	}
	else
	{
		ni.nameOffset = *(uint32_t*)(data + *offset);
		*offset += 4;
		ni.childrenOffset = *(uint32_t*)(data + *offset);
		*offset += 4;
	}

	ni.arraySize = *(int32_t*)(data + *offset);

	*offset += 4;

	memcpy_s(ni.extra, 12, data + *offset, 12);
	*offset += sizeof(ni.extra);

	if (is64)
	{
		ni.extra4 = *(uint64_t*)(data + *offset);
		*offset += 8;
	}
	else
	{
		ni.extra4 = *(uint32_t*)(data + *offset);
		*offset += 4;
	}

	*info = ni;
	return true;
}
