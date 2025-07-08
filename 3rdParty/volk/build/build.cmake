block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/volk/volk.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
    ${root}/volk/volk.c
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(volk)
settings3rdParty(volk)

target_compile_definitions(volk PUBLIC VK_USE_PLATFORM_WIN32_KHR)
# target_compile_definitions(volk PUBLIC VK_NO_PROTOTYPES)

target_include_directories(volk SYSTEM PUBLIC "${root}/volk")

target_link_libraries(volk PUBLIC
	vulkan_headers
)

endblock()