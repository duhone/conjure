block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

include (${root}/Packages/Core/build/build.cmake)
#include (${root}/Platform/build/build.cmake)

set(CR_INTERFACE_HEADERS
)

set(CR_INTERFACE_MODULES
    ${root}/interface/Engine.ixx
)

set(CR_IMPLEMENTATION
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(engine)
settingsCR(engine)

target_link_libraries(engine PUBLIC
    core
    platform
)

set_property(TARGET engine APPEND PROPERTY FOLDER Engine)

endblock()