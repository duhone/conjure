set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Guid.ixx
)

set(SOURCE_FILES
    ${root}/source/windows/Guid.cpp
    ${root}/source/windows/GuidImpl.h
    ${root}/source/windows/GuidImpl.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(platform 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
)

settingsCR(platform)

target_link_libraries(platform PUBLIC
	headerUnits
	core
)

set_property(TARGET platform APPEND PROPERTY FOLDER Engine)
	
###############################################
#unit tests
###############################################
set(SOURCE_FILES
	${root}/tests/main.cpp
)

add_executable(platform_tests 
	${SOURCE_FILES}
)
		
settingsCR(platform_tests)
	
target_link_libraries(platform_tests 
	headerUnits
	doctest
	platform
)		

set_property(TARGET platform_tests APPEND PROPERTY FOLDER Engine/Tests)
