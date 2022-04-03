set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
    ${root}/libsamplerate/include/samplerate.h
)

set(SOURCE_FILES
    ${root}/libsamplerate/src/common.h
    ${root}/libsamplerate/src/fastest_coeffs.h
    ${root}/libsamplerate/src/high_qual_coeffs.h
    ${root}/libsamplerate/src/mid_qual_coeffs.h
    ${root}/libsamplerate/src/samplerate.c
    ${root}/libsamplerate/src/src_linear.c
    ${root}/libsamplerate/src/src_sinc.c
    ${root}/libsamplerate/src/src_zoh.c
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(samplerate OBJECT 
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
)
settings3rdParty(samplerate)

target_compile_definitions(samplerate PRIVATE HAVE_CONFIG_H=1)

target_include_directories(samplerate SYSTEM PUBLIC "${root}/libsamplerate/include")
