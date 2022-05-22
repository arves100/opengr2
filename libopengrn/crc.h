/*!
	Project: libopengrn
	File: crc.h
	CRC32 functionalities

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <stdint.h>

extern uint32_t CRC32(const uint8_t* data, size_t len);
