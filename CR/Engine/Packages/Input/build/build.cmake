block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
)

set(CR_INTERFACE_MODULES
    ${root}/interface/Handles.ixx
    ${root}/interface/Input.ixx
    ${root}/interface/Regions.ixx
)

set(CR_IMPLEMENTATION
    ${root}/source/Input.cpp
    ${root}/source/Regions.ixx
    ${root}/source/Context.ixx
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(input)
settingsCR(input)

target_link_libraries(input PUBLIC
	glm
    glfw
	core
	platform
)

set_property(TARGET input APPEND PROPERTY FOLDER Engine/Packages)

endblock()