set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Input.ixx
    ${root}/interface/Regions.ixx
    ${root}/interface/Service.ixx
)

set(SOURCE_FILES
    ${root}/source/RegionService.ixx
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(input 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
)

settingsCR(input)

target_link_libraries(input PUBLIC
	headerUnits
	fmt
	function2
	glm
	spdlog
	core
	platform
)

set_property(TARGET input APPEND PROPERTY FOLDER Engine)

