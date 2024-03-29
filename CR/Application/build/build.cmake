set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
)

set(SOURCE_FILES
    ${root}/source/main.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_executable(application 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
)

settingsCR(application)

target_link_libraries(application PUBLIC
	headerUnits
	core
	platform
	audio
	input
	graphics
)

target_compile_definitions(application PRIVATE ASSETS_FOLDER="${root}/../Assets")