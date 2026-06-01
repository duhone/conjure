block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/reflect/reflect
)

set(CR_INTERFACE_MODULES
    ${root}/reflect/reflect.cppm
)

set(CR_IMPLEMENTATION
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(reflect)
settings3rdParty(reflect)

set_property(TARGET reflect APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(reflect SYSTEM PUBLIC "${root}/reflect")

endblock()