set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/simdjson/simdjson.h
)

set(SOURCE_FILES
    ${root}/simdjson/simdjson.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(simdjson OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(simdjson)

target_include_directories(simdjson SYSTEM PUBLIC "${root}/simdjson")

