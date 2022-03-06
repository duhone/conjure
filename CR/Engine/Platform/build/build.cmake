set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Guid.ixx
    ${root}/interface/MemoryMappedFile.ixx
    ${root}/interface/PathUtils.ixx
    ${root}/interface/Process.ixx
    ${root}/interface/platform/windows/CRWindows.h
)

set(SOURCE_FILES
    ${root}/source/windows/Guid.cpp
    ${root}/source/windows/MemoryMappedFile.cpp
    ${root}/source/windows/PathUtils.cpp
    ${root}/source/windows/Process.cpp
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

target_include_directories(platform SYSTEM PUBLIC "${root}/interface")

set_property(TARGET platform APPEND PROPERTY FOLDER Engine)
	
###############################################
#unit tests
###############################################
set(SOURCE_FILES
	${root}/tests/main.cpp
	${root}/tests/MemoryMappedFile.cpp
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

set(TEST_DATA 
	"${root}/tests/content/testdll.dll"
	"${root}/tests/content/test.txt"
)

add_custom_command(TARGET platform_tests POST_BUILD        
COMMAND ${CMAKE_COMMAND} -E copy_if_different  
	${TEST_DATA}
	$<TARGET_FILE_DIR:platform_tests>)