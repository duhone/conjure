block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

include (${root}/Packages/Embed/build/build.cmake)
include (${root}/Packages/AudioProcessor/build/build.cmake)

endblock()