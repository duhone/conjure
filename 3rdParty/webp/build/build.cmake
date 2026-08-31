block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/libwebp/src/webp/decode.h
    ${root}/libwebp/src/webp/types.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
    ${root}/libwebp/src/dec/alpha_dec.c
    ${root}/libwebp/src/dec/buffer_dec.c
    ${root}/libwebp/src/dec/frame_dec.c
    ${root}/libwebp/src/dec/idec_dec.c
    ${root}/libwebp/src/dec/io_dec.c
    ${root}/libwebp/src/dec/quant_dec.c
    ${root}/libwebp/src/dec/tree_dec.c
    ${root}/libwebp/src/dec/vp8_dec.c
    ${root}/libwebp/src/dec/vp8l_dec.c
    ${root}/libwebp/src/dec/webp_dec.c
    ${root}/libwebp/src/dsp/alpha_processing.c
    ${root}/libwebp/src/dsp/alpha_processing_sse2.c
    ${root}/libwebp/src/dsp/alpha_processing_sse41.c
    ${root}/libwebp/src/dsp/cpu.c
    ${root}/libwebp/src/dsp/dec.c
    ${root}/libwebp/src/dsp/dec_clip_tables.c
    ${root}/libwebp/src/dsp/dec_sse2.c
    ${root}/libwebp/src/dsp/dec_sse41.c
    ${root}/libwebp/src/dsp/filters.c
    ${root}/libwebp/src/dsp/filters_sse2.c
    ${root}/libwebp/src/dsp/lossless.c
    ${root}/libwebp/src/dsp/lossless_sse2.c
    ${root}/libwebp/src/dsp/lossless_sse41.c
    ${root}/libwebp/src/dsp/rescaler.c
    ${root}/libwebp/src/dsp/rescaler_sse2.c
    ${root}/libwebp/src/dsp/upsampling.c
    ${root}/libwebp/src/dsp/upsampling_sse2.c
    ${root}/libwebp/src/dsp/upsampling_sse41.c
    ${root}/libwebp/src/dsp/yuv.c
    ${root}/libwebp/src/dsp/yuv_sse2.c
    ${root}/libwebp/src/dsp/yuv_sse41.c
    ${root}/libwebp/src/utils/bit_reader_utils.c
    ${root}/libwebp/src/utils/color_cache_utils.c
    ${root}/libwebp/src/utils/filters_utils.c
    ${root}/libwebp/src/utils/huffman_utils.c
    ${root}/libwebp/src/utils/palette.c
    ${root}/libwebp/src/utils/quant_levels_dec_utils.c
    ${root}/libwebp/src/utils/random_utils.c
    ${root}/libwebp/src/utils/rescaler_utils.c
    ${root}/libwebp/src/utils/thread_utils.c
    ${root}/libwebp/src/utils/utils.c
    ${root}/libwebp/src/demux/anim_decode.c
    ${root}/libwebp/src/demux/demux.c
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(webp)
settings3rdParty(webp)

target_compile_definitions(webp PUBLIC WEBP_HAVE_SSE2=1)
target_compile_definitions(webp PUBLIC WEBP_HAVE_SSE41=1)
target_compile_definitions(webp PUBLIC WEBP_HAVE_AVX2=1)
target_compile_definitions(webp PUBLIC WEBP_USE_THREAD=1)

target_include_directories(webp SYSTEM PRIVATE "${root}/libwebp")
target_include_directories(webp SYSTEM PUBLIC "${root}/libwebp/src")

endblock()