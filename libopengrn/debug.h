/*!
	Project: libopengrn
	File: debug.h
	Debug utilities

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <stdio.h>
#include "elements.h"

#ifdef _DEBUG

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define DumpMemLeak() _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#else
#define DumpMemLeak()
#endif

extern void dbg_printf_real(const char* file, size_t line, const char* fmt, ...);
extern void dbg_printf_no_line(const char* fmt, ...);
extern void dbg_printelement(TElementGeneric* element);

#define dbg_printf2(fmt, ...) dbg_printf_real(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define dbg_printf(fmt, ...) dbg_printf_real(__FILE__, __LINE__, fmt "\n", ##__VA_ARGS__)
#define dbg_printf3(fmt, ...) dbg_printf_no_line(fmt, ##__VA_ARGS__)
#else
#define dbg_printf2(fmt, ...)
#define dbg_printf(fmt, ...)
#define dbg_printf3(fmt, ...)

#define DumpMemLeak()

#endif // _DEBUG
