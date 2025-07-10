block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/vulkan_headers/include/vulkan/vulkan.h
    ${root}/vulkan_headers/include/vulkan/vk_platform.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(vulkan_headers OBJECT)
settings3rdParty(vulkan_headers)

set_property(TARGET vulkan_headers APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(vulkan_headers SYSTEM PUBLIC "${root}/vulkan_headers/include")

endblock()