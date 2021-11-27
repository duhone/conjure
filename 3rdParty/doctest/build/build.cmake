set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/doctest/doctest/doctest.h
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(doctest OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(doctest)

set_property(TARGET doctest APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(doctest SYSTEM PUBLIC "${root}/doctest")
target_compile_definitions(doctest PUBLIC 
	DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
	DOCTEST_CONFIG_USE_STD_HEADERS 
	DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#	DOCTEST_CONFIG_NO_EXCEPTIONS
)
