set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/volk/volk.h
)

set(SOURCE_FILES
    ${root}/volk/volk.c
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(volk OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(volk)

target_compile_definitions(volk PUBLIC VK_USE_PLATFORM_WIN32_KHR)
# target_compile_definitions(volk PUBLIC VK_NO_PROTOTYPES)

target_include_directories(volk SYSTEM PUBLIC "${root}/volk")

target_link_libraries(volk PUBLIC
	vulkan_headers
)
