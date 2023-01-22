set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Graphics.ixx
    ${root}/interface/Service.ixx
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(graphics 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
)

settingsCR(graphics)

target_link_libraries(graphics PUBLIC
	headerUnits
	fmt
	glm
	spdlog
	core
	platform
)

set_property(TARGET graphics APPEND PROPERTY FOLDER Engine)

