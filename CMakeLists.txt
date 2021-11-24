cmake_minimum_required (VERSION 3.21)
set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Profile;Final") 

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project (Conjure)

include (${CMAKE_CURRENT_SOURCE_DIR}/common.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/doctest/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/CR/Engine/Core/build/build.cmake)
	