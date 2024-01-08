set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/unordered_dense/include/ankerl/unordered_dense.h
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(unordered_dense OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(unordered_dense)

set_property(TARGET unordered_dense APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(unordered_dense SYSTEM PUBLIC "${root}/unordered_dense/include")

