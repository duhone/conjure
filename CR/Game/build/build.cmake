block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
    ${root}/source/main.cpp
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

set(CR_SCHEMA_FILES
)

set(CR_GENERATED_FILES
)

add_executable(game 
)

settingsCR(game)

target_link_libraries(game PUBLIC
	engine
)

target_compile_definitions(game PRIVATE ASSETS_FOLDER="${root}/../Assets")

endblock()