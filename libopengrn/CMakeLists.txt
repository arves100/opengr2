set(SOURCE_FILES
        compression.c
        darray.c
        debug.c
        elements.c
        gr2.c
        platform.c
        magic.c
        oodle1.c
        virtual_ptr.c
)

set(HEADER_FILES
        compression.h
        darray.h
        debug.h
        dllapi.h
        elements.h
        gr2.h
        magic.h
        platform.h
        structures.h
        oodle1.h
        virtual_ptr.h
)

if (NOT OPENGRN_STATIC)
        set(LIB_VARIANT SHARED)
else()
        set(LIB_VARIANT STATIC)
endif()

add_library(libopengrn STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(libopengrn PUBLIC .)

if (NOT OPENGRN_STATIC)
        target_compile_definitions(libopengrn PRIVATE -DBUILD_LIBOPENGRN)
else()
        target_compile_definitions(libopengrn PUBLIC -DBUILD_LIBOPENGRN_STATIC)
endif()
