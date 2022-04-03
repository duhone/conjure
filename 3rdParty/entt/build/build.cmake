set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/entt/entt/entt.hpp
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(entt OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(entt)

set_property(TARGET entt APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(entt SYSTEM PUBLIC "${root}/entt")
