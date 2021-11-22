set(root "${CMAKE_CURRENT_LIST_DIR}/../catch")

set(PUBLIC_INTERFACE
    ${root}/catch_amalgamated.hpp
)

set(SRCS
    ${root}/catch_amalgamated.cpp
)

set(BUILD
    ${root}/../build/catch.cmake
)

add_library(catch OBJECT 
	${PUBLIC_INTERFACE} 
	${SRCS} 
	${BUILD}
)
settings3rdParty(catch)

target_include_directories(catch SYSTEM PUBLIC "${root}")
