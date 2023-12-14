set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/VulkanMemoryAllocator/include/vk_mem_alloc.h
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(vulkan_vma OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(vulkan_vma)

set_property(TARGET vulkan_vma APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(vulkan_vma SYSTEM PUBLIC "${root}/VulkanMemoryAllocator/include")

target_link_libraries(vulkan_vma PUBLIC
	vulkan_headers
)
