set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/cli11/CLI11.hpp
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(cli11 OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(cli11)

set_property(TARGET cli11 APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(cli11 SYSTEM PUBLIC "${root}")
