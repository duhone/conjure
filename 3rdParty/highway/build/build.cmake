block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")
# at the moment we only care about decoding

set(CR_INTERFACE_HEADERS
    ${root}/highway/hwy/highway.h
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
# base files
    ${root}/highway/hwy/aligned_allocator.cc
    ${root}/highway/hwy/aligned_allocator.h
    ${root}/highway/hwy/base.h
    ${root}/highway/hwy/cache_control.h
    ${root}/highway/hwy/detect_compiler_arch.h  # private
    ${root}/highway/hwy/detect_targets.h  # private
    ${root}/highway/hwy/foreach_target.h
    ${root}/highway/hwy/highway_export.h
    ${root}/highway/hwy/nanobenchmark.cc
    ${root}/highway/hwy/nanobenchmark.h
    ${root}/highway/hwy/ops/arm_neon-inl.h
    ${root}/highway/hwy/ops/arm_sve-inl.h
    ${root}/highway/hwy/ops/emu128-inl.h
    ${root}/highway/hwy/ops/generic_ops-inl.h
    ${root}/highway/hwy/ops/ppc_vsx-inl.h
    ${root}/highway/hwy/ops/rvv-inl.h
    ${root}/highway/hwy/ops/scalar-inl.h
    ${root}/highway/hwy/ops/set_macros-inl.h
    ${root}/highway/hwy/ops/shared-inl.h
    ${root}/highway/hwy/ops/wasm_128-inl.h
    ${root}/highway/hwy/ops/tuple-inl.h
    ${root}/highway/hwy/ops/x86_128-inl.h
    ${root}/highway/hwy/ops/x86_256-inl.h
    ${root}/highway/hwy/ops/x86_512-inl.h
    ${root}/highway/hwy/per_target.cc
    ${root}/highway/hwy/per_target.h
    ${root}/highway/hwy/print-inl.h
    ${root}/highway/hwy/print.cc
    ${root}/highway/hwy/print.h
    ${root}/highway/hwy/robust_statistics.h
    ${root}/highway/hwy/targets.cc
    ${root}/highway/hwy/targets.h
    ${root}/highway/hwy/timer.cc
    ${root}/highway/hwy/timer.h
    ${root}/highway/hwy/timer-inl.h
# contrib
    ${root}/highway/hwy/contrib/dot/dot-inl.h
    ${root}/highway/hwy/contrib/image/image.cc
    ${root}/highway/hwy/contrib/image/image.h
    ${root}/highway/hwy/contrib/math/math-inl.h
    ${root}/highway/hwy/contrib/sort/order.h
    ${root}/highway/hwy/contrib/sort/shared-inl.h
    ${root}/highway/hwy/contrib/sort/sorting_networks-inl.h
    ${root}/highway/hwy/contrib/sort/traits-inl.h
    ${root}/highway/hwy/contrib/sort/traits128-inl.h
    ${root}/highway/hwy/contrib/sort/vqsort-inl.h
    ${root}/highway/hwy/contrib/sort/vqsort.cc
    ${root}/highway/hwy/contrib/sort/vqsort.h
    ${root}/highway/hwy/contrib/algo/copy-inl.h
    ${root}/highway/hwy/contrib/algo/find-inl.h
    ${root}/highway/hwy/contrib/algo/transform-inl.h
    ${root}/highway/hwy/contrib/unroller/unroller-inl.h
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(highway)
settings3rdParty(highway)

target_include_directories(highway SYSTEM PUBLIC "${root}/highway")

endblock()