set(root "${CMAKE_CURRENT_LIST_DIR}/..")

# too lazy to list all the headers
set(INTERFACE_FILES
    ${root}/glm/glm/glm.hpp
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(glm OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(glm)

set_property(TARGET glm APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(glm SYSTEM PUBLIC "${root}/glm")
