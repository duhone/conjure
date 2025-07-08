block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/vulkan_utilities/include/vulkan/vk_enum_string_helper.h
    ${root}/vulkan_utilities/include/vulkan/utility/vk_format_utils.h
    ${root}/vulkan_utilities/include/vulkan/utility/vk_struct_helper.hpp
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(vulkan_utilities)
settings3rdParty(vulkan_utilities)

set_property(TARGET vulkan_utilities APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(vulkan_utilities SYSTEM PUBLIC "${root}/vulkan_utilities/include")

target_link_libraries(vulkan_utilities PUBLIC
	vulkan_headers
)

endblock()