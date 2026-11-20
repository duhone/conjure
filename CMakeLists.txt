cmake_minimum_required (VERSION 4.5 FATAL_ERROR)

set(root "${CMAKE_CURRENT_LIST_DIR}")

set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Profile;Final") 

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project (Conjure LANGUAGES CXX C)
  
include (${root}/common.cmake)
include (${root}/3rdParty/build/build.cmake)
include (${root}/Engine/build/build.cmake)
include (${root}/Tools/build/build.cmake)
	