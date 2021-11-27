set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/function2/include/function2/function2.hpp
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(function2 OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(function2)

set_property(TARGET function2 APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(function2 SYSTEM PUBLIC "${root}/function2/include")
