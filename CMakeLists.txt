cmake_minimum_required (VERSION 3.22)
set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Profile;Final") 

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project (Conjure)
  
include (${CMAKE_CURRENT_SOURCE_DIR}/common.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/cli11/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/doctest/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/fmt/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/function2/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glm/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/opus/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/spdlog/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/zstd/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/HeaderUnits/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/CR/Engine/Core/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/CR/Engine/DataCompression/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/CR/Engine/Platform/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/CR/Tools/embed/build/build.cmake)
	