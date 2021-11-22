set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Algorithm.ixx
    ${root}/interface/FileHandle.ixx
    ${root}/interface/TypeTraits.ixx
)

set(SOURCE_FILES
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(core 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
)

settingsCR(core)

target_link_libraries(core PUBLIC)

set_property(TARGET core APPEND PROPERTY FOLDER Engine)
	
###############################################
#unit tests
###############################################
set(SOURCE_FILES
	${root}/tests/TypeTraits.cpp
)

add_executable(core_tests 
	${SOURCE_FILES}
)
		
settingsCR(core_tests)
	
target_link_libraries(core_tests 
	catch
	core
)		

set_property(TARGET core_tests APPEND PROPERTY FOLDER Engine/Tests)
