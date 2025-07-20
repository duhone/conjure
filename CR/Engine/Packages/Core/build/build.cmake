block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
    ${root}/interface/core/Core.h
    ${root}/interface/core/Defer.h
    ${root}/interface/core/Log.h
    ${root}/interface/core/Reflection.h
)

set(CR_INTERFACE_MODULES
    ${root}/interface/Algorithm.ixx
    ${root}/interface/BinaryStream.ixx
    ${root}/interface/BitSet.ixx
    ${root}/interface/Core.ixx
    ${root}/interface/EightCC.ixx
    ${root}/interface/Embedded.ixx
    ${root}/interface/FileHandle.ixx
    ${root}/interface/Function.ixx
    ${root}/interface/Guid.ixx
    ${root}/interface/Handle.ixx
    ${root}/interface/Hash.ixx
    ${root}/interface/Literals.ixx
    ${root}/interface/Locked.ixx
    ${root}/interface/Log.ixx
    ${root}/interface/Random.ixx
    ${root}/interface/Rect.ixx
    ${root}/interface/Defer.ixx
    ${root}/interface/TypeTraits.ixx
)

set(CR_IMPLEMENTATION
    ${root}/implementation/Log.cpp
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

add_library(core)
settingsCR(core)

target_link_libraries(core PUBLIC
    glm
    spdlog
)

target_include_directories(core SYSTEM PUBLIC "${root}/interface")

set_property(TARGET core APPEND PROPERTY FOLDER Engine/Packages)
	
###############################################
#unit tests
###############################################

set(CR_INTERFACE_HEADERS
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
	${root}/tests/BinaryStream.cpp
	${root}/tests/BitSet.cpp
	${root}/tests/Function.cpp
	${root}/tests/Guid.cpp
	${root}/tests/Locked.cpp
    ${root}/tests/Log.cpp
	${root}/tests/main.cpp
	${root}/tests/TypeTraits.cpp
)

add_executable(core_tests)
		
settingsCR(core_tests)
	
target_link_libraries(core_tests 
	doctest
	core
)		

set_property(TARGET core_tests APPEND PROPERTY FOLDER Engine/Tests)

endblock()