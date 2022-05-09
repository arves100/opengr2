cmake_minimum_required(VERSION 3.22)
project(opengr2)

#add_definitions(-D_DEBUG)

set(SOURCE_FILES
        gr2nfo/gr2nfo.c
        libopengrn/compression.c
        libopengrn/darray.c
        libopengrn/debug.c
        libopengrn/elements.c
        libopengrn/gr2.c
        libopengrn/platform.c
        libopengrn/magic.c
        libopengrn/oodle1.c libopengrn/virtual_ptr.c)

set(HEADER_FILES
        libopengrn/compression.h
        libopengrn/darray.h
        libopengrn/debug.h
        libopengrn/dllapi.h
        libopengrn/elements.h
        libopengrn/gr2.h
        libopengrn/magic.h
        libopengrn/platform.h
        libopengrn/structures.h
        libopengrn/oodle1.h libopengrn/virtual_ptr.h)

add_library(opengr2 STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(opengr2 PUBLIC .)