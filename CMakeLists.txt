cmake_minimum_required (VERSION 4.0 FATAL_ERROR)

set(root "${CMAKE_CURRENT_LIST_DIR}")

set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Profile;Final") 
# because of this, cmake 4.0.3 is required at the moment. have to update uuid if changing versions.
# get uuid from here https://github.com/Kitware/CMake/blob/v4.0.3/Help/dev/experimental.rst
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project (Conjure LANGUAGES CXX C)
  
include (${root}/common.cmake)
include (${root}/3rdParty/build/build.cmake)
#include (${root}/CR/Engine/build/build.cmake)
#include (${root}/CR/Tools/build/build.cmake)
#include (${root}/CR/Application/build/build.cmake)

#include (${root}/3rdParty/entt/build/build.cmake)
#include (${root}/3rdParty/flatbuffers/build/build.cmake)
#include (${root}/3rdParty/fmt/build/build.cmake)
#include (${root}/3rdParty/function2/build/build.cmake)
#include (${root}/3rdParty/glm/build/build.cmake)
#include (${root}/3rdParty/highway/build/build.cmake)
#include (${root}/3rdParty/libsamplerate/build/build.cmake)
#include (${root}/3rdParty/libjxl/build/build.cmake)
#include (${root}/3rdParty/opus/build/build.cmake)
#include (${root}/3rdParty/spdlog/build/build.cmake)
#include (${root}/3rdParty/unordered_dense/build/build.cmake)
#include (${root}/3rdParty/vulkan_headers/build/build.cmake)
#include (${root}/3rdParty/vulkan_utilities/build/build.cmake)
#include (${root}/3rdParty/vulkan_vma/build/build.cmake)
#include (${root}/3rdParty/volk/build/build.cmake)
#include (${root}/3rdParty/zstd/build/build.cmake)
#include (${root}/CR/Engine/Core/build/build.cmake)
#include (${root}/CR/Engine/Assets/build/build.cmake)
#include (${root}/CR/Engine/Audio/build/build.cmake)
#include (${root}/CR/Engine/Compression/build/build.cmake)
#include (${root}/CR/Engine/Input/build/build.cmake)
#include (${root}/CR/Engine/Platform/build/build.cmake)
#include (${root}/CR/Engine/Graphics/build/build.cmake)
#include (${root}/CR/Tools/AudioProcessor/build/build.cmake)
#include (${root}/CR/Tools/embed/build/build.cmake)
#include (${root}/CR/Application/build/build.cmake)
	