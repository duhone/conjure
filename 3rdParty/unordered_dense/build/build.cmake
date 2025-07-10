block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/unordered_dense/include/ankerl/unordered_dense.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(unordered_dense OBJECT)
settings3rdParty(unordered_dense)

set_property(TARGET unordered_dense APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(unordered_dense SYSTEM PUBLIC "${root}/unordered_dense/include")

endblock()