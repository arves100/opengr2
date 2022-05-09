/*!
	Project: libopengrn
	File: dllapi.h
	DLL Export

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#pragma once

#if !defined(BUILD_LIBOPENGRN_STATIC) && defined(_MSC_VER)

#ifdef BUILD_LIBOPENGRN
#define OG_DLLAPI __declspec(dllexport)
#else
#define OG_DLLAPI __declspec(dllimport)
#endif

#else
#define OG_DLLAPI 
#endif
