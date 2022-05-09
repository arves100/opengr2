cmake_minimum_required(VERSION 3.22)
project(opengr2)

option(OPENGRN_STATIC "Build libopengrn as a static library" ON)

add_subdirectory(libopengrn)
add_subdirectory(gr2nfo)
