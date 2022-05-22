set(SOURCE_FILES
        compression.c
        darray.c
        debug.c
        elements.c
        elements_parse.c
        gr2.c
        gr2_read.c
        gr2_write.c
        platform.c
        magic.c
        oodle1.c
        virtual_ptr.c
        typeinfo.c
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
        typeinfo.h
)

if (NOT OPENGRN_STATIC)
        set(LIB_VARIANT SHARED)
else()
        set(LIB_VARIANT STATIC)
endif()

add_library(opengrn STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(opengrn PUBLIC .)

if (NOT OPENGRN_STATIC)
        target_compile_definitions(opengrn PRIVATE -DBUILD_LIBOPENGRN)
else()
        target_compile_definitions(opengrn PUBLIC -DBUILD_LIBOPENGRN_STATIC)
endif()
