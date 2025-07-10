block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/cli11/CLI11.hpp
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(cli11 OBJECT)
settings3rdParty(cli11)

set_property(TARGET cli11 APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(cli11 SYSTEM PUBLIC "${root}")

endblock()