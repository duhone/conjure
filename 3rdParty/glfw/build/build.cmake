block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")
# at the moment we only care about decoding

set(CR_INTERFACE_HEADERS
    ${root}/glfw/include/GLFW/glfw3.h
    ${root}/glfw/include/GLFW/glfw3native.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
    ${root}/glfw/src/win32_time.h
    ${root}/glfw/src/win32_thread.h
    ${root}/glfw/src/win32_module.c
    ${root}/glfw/src/win32_time.c
    ${root}/glfw/src/win32_thread.c
    ${root}/glfw/src/internal.h
    ${root}/glfw/src/platform.h
    ${root}/glfw/src/mappings.h
    ${root}/glfw/src/context.c
    ${root}/glfw/src/init.c
    ${root}/glfw/src/input.c
    ${root}/glfw/src/monitor.c
    ${root}/glfw/src/platform.c
    ${root}/glfw/src/vulkan.c
    ${root}/glfw/src/window.c
    ${root}/glfw/src/egl_context.c
    ${root}/glfw/src/osmesa_context.c
    ${root}/glfw/src/null_platform.h
    ${root}/glfw/src/null_joystick.h
    ${root}/glfw/src/null_init.c
    ${root}/glfw/src/null_monitor.c
    ${root}/glfw/src/null_window.c
    ${root}/glfw/src/null_joystick.c
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(glfw)

settings3rdParty(glfw)

target_compile_definitions(glfw PRIVATE _GLFW_WIN32)
target_compile_definitions(glfw PRIVATE UNICODE _UNICODE)
target_compile_definitions(glfw PRIVATE _CRT_SECURE_NO_WARNINGS)

target_include_directories(glfw SYSTEM PUBLIC "${root}/glfw/include")

endblock()