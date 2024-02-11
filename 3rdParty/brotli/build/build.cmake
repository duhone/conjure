set(root "${CMAKE_CURRENT_LIST_DIR}/..")
# at the moment we only care about decoding

set(INTERFACE_FILES
    ${root}/brotli/c/include/brotli/decode.h
    ${root}/brotli/c/include/brotli/port.h
    ${root}/brotli/c/include/brotli/shared_dictionary.h
    ${root}/brotli/c/include/brotli/types.h
)

set(SOURCE_FILES
    ${root}/brotli/c/common/constants.h
    ${root}/brotli/c/common/constants.c
    ${root}/brotli/c/common/context.h
    ${root}/brotli/c/common/context.c
    ${root}/brotli/c/common/dictionary.h
    ${root}/brotli/c/common/dictionary.c
    ${root}/brotli/c/common/platform.h
    ${root}/brotli/c/common/platform.c
    ${root}/brotli/c/common/shared_dictionary_internal.h
    ${root}/brotli/c/common/shared_dictionary.c
    ${root}/brotli/c/common/transform.h
    ${root}/brotli/c/common/transform.c
    ${root}/brotli/c/common/version.h
    ${root}/brotli/c/dec/bit_reader.h
    ${root}/brotli/c/dec/bit_reader.c
    ${root}/brotli/c/dec/decode.c
    ${root}/brotli/c/dec/huffman.h
    ${root}/brotli/c/dec/huffman.c
    ${root}/brotli/c/dec/prefix.h
    ${root}/brotli/c/dec/state.h
    ${root}/brotli/c/dec/state.c
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(brotli OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(brotli)

target_include_directories(brotli SYSTEM PUBLIC "${root}/brotli/c/include")
