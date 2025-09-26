block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(FLATC "${root}/flatbuffers/bin/flatc.exe" CACHE INTERNAL "flatc path")

set(CR_INTERFACE_HEADERS
    ${root}/flatbuffers/include/flatbuffers/flatbuffers.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
    ${root}/flatbuffers/src/idl_parser.cpp
    ${root}/flatbuffers/src/util.cpp
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(flatbuffers)
settings3rdParty(flatbuffers)

set_property(TARGET flatbuffers APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(flatbuffers SYSTEM PUBLIC "${root}/flatbuffers/include")
target_compile_definitions(flatbuffers PUBLIC FLATC_PATH=${FLATC})

endblock()