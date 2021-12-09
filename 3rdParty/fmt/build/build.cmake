set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/fmt/include/fmt/args.h
    ${root}/fmt/include/fmt/chrono.h
    ${root}/fmt/include/fmt/color.h
    ${root}/fmt/include/fmt/compile.h
    ${root}/fmt/include/fmt/core.h
    ${root}/fmt/include/fmt/format.h
    ${root}/fmt/include/fmt/format-inl.h
    ${root}/fmt/include/fmt/locale.h
    ${root}/fmt/include/fmt/os.h
    ${root}/fmt/include/fmt/ostream.h
    ${root}/fmt/include/fmt/printf.h
    ${root}/fmt/include/fmt/ranges.h
    ${root}/fmt/include/fmt/xchar.h
)

set(SOURCE_FILES
    ${root}/fmt/src/format.cc
    ${root}/fmt/src/os.cc
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(fmt OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(fmt)

set_property(TARGET fmt APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(fmt SYSTEM PUBLIC "${root}/fmt/include")
