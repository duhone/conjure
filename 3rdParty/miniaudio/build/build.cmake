block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")
# at the moment we only care about decoding

set(CR_INTERFACE_HEADERS
    ${root}/miniaudio/miniaudio.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
    ${root}/miniaudio/miniaudio.c
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(miniaudio)

settings3rdParty(miniaudio)

target_compile_definitions(miniaudio PUBLIC MA_ENABLE_ONLY_SPECIFIC_BACKENDS)
target_compile_definitions(miniaudio PUBLIC MA_ENABLE_WASAPI)
target_compile_definitions(miniaudio PUBLIC MA_NO_ENCODING)
target_compile_definitions(miniaudio PUBLIC MA_NO_MP3)
target_compile_definitions(miniaudio PUBLIC MA_NO_RESOURCE_MANAGER)

target_include_directories(miniaudio SYSTEM PUBLIC "${root}/miniaudio")

endblock()