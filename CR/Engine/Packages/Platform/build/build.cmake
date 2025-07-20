block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
)

set(CR_INTERFACE_MODULES
    ${root}/interface/Guid.ixx
    ${root}/interface/MemoryMappedFile.ixx
    ${root}/interface/PathUtils.ixx
    ${root}/interface/Platform.ixx
    ${root}/interface/Process.ixx
    ${root}/interface/SharedLibrary.ixx
)

set(CR_IMPLEMENTATION
    ${root}/source/windows/CRWindows.h
    ${root}/source/windows/Guid.cpp
    ${root}/source/windows/MemoryMappedFile.cpp
    ${root}/source/windows/PathUtils.cpp
    ${root}/source/windows/Process.cpp
    ${root}/source/windows/SharedLibrary.cpp
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(platform)
settingsCR(platform)

#microsoft bug see https://developercommunity.visualstudio.com/t/warning-C4005:-Outptr:-macro-redefinit/1546919
target_compile_options(platform PRIVATE /WX-)

target_link_libraries(platform PUBLIC
    glm
	core
)

set_property(TARGET platform APPEND PROPERTY FOLDER Engine/Packages)
	
###############################################
#unit tests
###############################################
set(CR_INTERFACE_HEADERS
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
	${root}/tests/main.cpp
	${root}/tests/MemoryMappedFile.cpp
	${root}/tests/SharedLibrary.cpp
)

add_executable(platform_tests)		
settingsCR(platform_tests)
	
target_link_libraries(platform_tests 
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
    
endblock()