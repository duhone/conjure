set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/interface/catch.h
)

set(SOURCE_FILES
    ${root}/catch/catch_amalgamated.hpp
    ${root}/catch/catch_amalgamated.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(catch OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(catch)

target_include_directories(catch SYSTEM PUBLIC "${root}/interface")
