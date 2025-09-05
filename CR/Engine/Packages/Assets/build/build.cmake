block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
)

set(CR_INTERFACE_MODULES
    ${root}/interface/Assets.ixx
)

set(CR_IMPLEMENTATION
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

set(CR_SCHEMA_FILES
)

set(CR_GENERATED_FILES
)

add_library(assets)
settingsCR(assets)

target_link_libraries(assets PUBLIC
	flatbuffers
	unordered_dense
	core
	platform
)

set_property(TARGET assets APPEND PROPERTY FOLDER Engine/Packages)

endblock()