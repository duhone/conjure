block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/spdlog/include/spdlog/async.h
    ${root}/spdlog/include/spdlog/async_logger.h
    ${root}/spdlog/include/spdlog/common.h
    ${root}/spdlog/include/spdlog/formatter.h
    ${root}/spdlog/include/spdlog/fwd.h
    ${root}/spdlog/include/spdlog/logger.h
    ${root}/spdlog/include/spdlog/pattern_formatter.h
    ${root}/spdlog/include/spdlog/spdlog.h
    ${root}/spdlog/include/spdlog/version.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
    ${root}/spdlog/src/async.cpp
    ${root}/spdlog/src/cfg.cpp
    ${root}/spdlog/src/color_sinks.cpp
    ${root}/spdlog/src/file_sinks.cpp
    ${root}/spdlog/src/spdlog.cpp
    ${root}/spdlog/src/stdout_sinks.cpp
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(spdlog)
settings3rdParty(spdlog)

set_property(TARGET spdlog APPEND PROPERTY LINKER_LANGUAGE CPP)

target_compile_definitions(spdlog PUBLIC SPDLOG_COMPILED_LIB)
target_compile_definitions(spdlog PUBLIC SPDLOG_USE_STD_FORMAT)

target_include_directories(spdlog SYSTEM PUBLIC "${root}/spdlog/include")

endblock()