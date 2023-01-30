set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/drlibs/dr_flac.h
)

set(SOURCE_FILES
    ${root}/source/drlibs.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(drlibs OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(drlibs)

set_property(TARGET drlibs APPEND PROPERTY LINKER_LANGUAGE CPP)

target_compile_definitions(drlibs PUBLIC DR_FLAC_NO_STDIO)

target_include_directories(drlibs SYSTEM PUBLIC "${root}/drlibs")

