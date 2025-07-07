block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

# too lazy to list all the headers
set(CR_INTERFACE_HEADERS
    ${root}/glm/glm/glm.hpp
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(glm)
settings3rdParty(glm)

set_property(TARGET glm APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(glm SYSTEM PUBLIC "${root}/glm")

endblock()