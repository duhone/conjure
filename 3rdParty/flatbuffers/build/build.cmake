set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(FLATC "${root}/flatbuffers/bin/flatc.exe")

set(INTERFACE_FILES
    ${root}/flatbuffers/include/flatbuffers/flatbuffers.h
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(flatbuffers OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(flatbuffers)

set_property(TARGET flatbuffers APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(flatbuffers SYSTEM PUBLIC "${root}/flatbuffers/include")

