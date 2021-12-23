/*!
	Project: libopengrn
	File: dllapi.h
	DLL Export

	Copyright (C) 2021 Arves100
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
