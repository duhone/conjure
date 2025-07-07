block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/drlibs/dr_flac.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
    ${root}/source/drlibs.cpp
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(drlibs)
settings3rdParty(drlibs)

set_property(TARGET drlibs APPEND PROPERTY LINKER_LANGUAGE CPP)

target_compile_definitions(drlibs PUBLIC DR_FLAC_NO_STDIO)

target_include_directories(drlibs SYSTEM PUBLIC "${root}/drlibs")

endblock()