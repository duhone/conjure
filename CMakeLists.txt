cmake_minimum_required (VERSION 4.0 FATAL_ERROR)

set(root "${CMAKE_CURRENT_LIST_DIR}")

set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Profile;Final") 
# because of this, cmake 4.0.4-4.2.3 is required at the moment. might have to update uuid if changing versions.
# get uuid from here https://github.com/Kitware/CMake/blob/v4.2.3/Help/dev/experimental.rst
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project (Conjure LANGUAGES CXX C)
  
include (${root}/common.cmake)
include (${root}/3rdParty/build/build.cmake)
include (${root}/Engine/build/build.cmake)
include (${root}/Tools/build/build.cmake)
	