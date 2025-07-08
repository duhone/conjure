block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/VulkanMemoryAllocator/include/vk_mem_alloc.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(vulkan_vma)
settings3rdParty(vulkan_vma)

set_property(TARGET vulkan_vma APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(vulkan_vma SYSTEM PUBLIC "${root}/VulkanMemoryAllocator/include")

target_link_libraries(vulkan_vma PUBLIC
	vulkan_headers
)

endblock()