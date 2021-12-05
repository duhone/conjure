set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
)

set(SOURCE_FILES
    ${root}/source/HeaderUnits.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(headerUnits 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
)

settingsCR(headerUnits)

target_link_libraries(headerUnits
	function2
)

set_property(TARGET headerUnits APPEND PROPERTY FOLDER HeaderUnits)
set_property(TARGET headerUnits APPEND PROPERTY VS_USER_PROPS ${root}/HeaderUnits.props)
	
