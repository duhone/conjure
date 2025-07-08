
block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

include (${root}/brotli/build/build.cmake)
include (${root}/cli11/build/build.cmake)
include (${root}/doctest/build/build.cmake)
include (${root}/drlibs/build/build.cmake)
include (${root}/flatbuffers/build/build.cmake)
include (${root}/glm/build/build.cmake)
include (${root}/highway/build/build.cmake)
include (${root}/libjxl/build/build.cmake)
include (${root}/libsamplerate/build/build.cmake)
include (${root}/opus/build/build.cmake)
include (${root}/spdlog/build/build.cmake)

endblock()