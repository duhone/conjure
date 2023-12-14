set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/vulkan_headers/include/vulkan/vulkan.h
    ${root}/vulkan_headers/include/vulkan/vk_platform.h
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(vulkan_headers OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(vulkan_headers)

set_property(TARGET vulkan_headers APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(vulkan_headers SYSTEM PUBLIC "${root}/vulkan_headers/include")

target_link_libraries(vulkan_headers PUBLIC
)
