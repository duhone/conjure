set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Assets.ixx
    ${root}/interface/Service.ixx
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(assets 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
)

settingsCR(assets)

target_link_libraries(assets PUBLIC
	headerUnits
	flatbuffers
	fmt
	unordered_dense
	core
	platform
)

set_property(TARGET assets APPEND PROPERTY FOLDER Engine)

