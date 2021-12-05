set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Algorithm.ixx
    ${root}/interface/BinaryStream.ixx
    ${root}/interface/FileHandle.ixx
    ${root}/interface/Function.ixx
    ${root}/interface/Guid.ixx
    ${root}/interface/Hash.ixx
    ${root}/interface/Literals.ixx
    ${root}/interface/Locked.ixx
    ${root}/interface/Random.ixx
    ${root}/interface/ScopeExit.ixx
    ${root}/interface/TypeTraits.ixx
)

set(SOURCE_FILES
    ${root}/source/Hash.cpp
    ${root}/source/Random.cpp
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

target_link_libraries(core PUBLIC
	headerUnits
	function2
)

set_property(TARGET core APPEND PROPERTY FOLDER Engine)
	
###############################################
#unit tests
###############################################
set(SOURCE_FILES
	${root}/tests/BinaryStream.cpp
	${root}/tests/Function.cpp
	${root}/tests/Guid.cpp
	${root}/tests/Locked.cpp
	${root}/tests/main.cpp
	${root}/tests/TypeTraits.cpp
)

add_executable(core_tests 
	${SOURCE_FILES}
)
		
settingsCR(core_tests)
	
target_link_libraries(core_tests 
	headerUnits
	doctest
	core
	function2
)		

set_property(TARGET core_tests APPEND PROPERTY FOLDER Engine/Tests)
