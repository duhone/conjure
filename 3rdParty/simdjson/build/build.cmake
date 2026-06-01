block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/simdjson/simdjson.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
    ${root}/simdjson/simdjson.cpp
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(simdjson)
settings3rdParty(simdjson)

target_include_directories(simdjson SYSTEM PUBLIC "${root}/simdjson")

endblock()