set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/zstd/lib/zstd.h
)

set(SOURCE_FILES
    ${root}/zstd/lib/common/debug.c
    ${root}/zstd/lib/common/entropy_common.c
    ${root}/zstd/lib/common/error_private.c
    ${root}/zstd/lib/common/fse_decompress.c
    ${root}/zstd/lib/common/pool.c
    ${root}/zstd/lib/common/threading.c
    ${root}/zstd/lib/common/xxhash.c
    ${root}/zstd/lib/common/zstd_common.c
    ${root}/zstd/lib/compress/fse_compress.c
    ${root}/zstd/lib/compress/hist.c
    ${root}/zstd/lib/compress/huf_compress.c
    ${root}/zstd/lib/compress/zstd_compress.c
    ${root}/zstd/lib/compress/zstd_compress_literals.c
    ${root}/zstd/lib/compress/zstd_compress_sequences.c
    ${root}/zstd/lib/compress/zstd_compress_superblock.c
    ${root}/zstd/lib/compress/zstd_double_fast.c
    ${root}/zstd/lib/compress/zstd_fast.c
    ${root}/zstd/lib/compress/zstd_lazy.c
    ${root}/zstd/lib/compress/zstd_ldm.c
    ${root}/zstd/lib/compress/zstd_opt.c
    ${root}/zstd/lib/compress/zstdmt_compress.c
    ${root}/zstd/lib/decompress/huf_decompress.c
    ${root}/zstd/lib/decompress/zstd_ddict.c
    ${root}/zstd/lib/decompress/zstd_decompress.c
    ${root}/zstd/lib/decompress/zstd_decompress_block.c
    ${root}/zstd/lib/dictBuilder/cover.c
    ${root}/zstd/lib/dictBuilder/divsufsort.c
    ${root}/zstd/lib/dictBuilder/fastcover.c
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(zstd OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(zstd)

target_compile_definitions(zstd PRIVATE XXH_NAMESPACE=ZSTD_)
target_compile_definitions(zstd PRIVATE ZSTD_LEGACY_SUPPORT=0)

set_property(TARGET zstd APPEND PROPERTY LINKER_LANGUAGE CPP)

target_include_directories(zstd SYSTEM PRIVATE "${root}/zstd/lib/common")
target_include_directories(zstd SYSTEM PUBLIC "${root}/zstd/lib")
