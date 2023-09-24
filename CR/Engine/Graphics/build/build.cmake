set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Graphics.ixx
    ${root}/interface/Service.ixx
)

set(SOURCE_FILES
    ${root}/source/Core.h
    ${root}/source/Constants.ixx
    ${root}/source/Context.ixx
    ${root}/source/DeviceService.ixx
    ${root}/source/CommandPool.ixx
    ${root}/source/Commands.ixx
    ${root}/source/Materials.ixx
    ${root}/source/Shaders.ixx
    ${root}/source/Utils.ixx
    ${root}/source/VkMemAllocator.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

set(SHADER_FILES
    ${assets_root}/Graphics/Shaders/sprite.comp
    ${assets_root}/Graphics/Shaders/sprite.vert
    ${assets_root}/Graphics/Shaders/sprite.frag
)

set(SCHEMA_FILES
    ${root}/source/schemas/materials.fbs
)

set(GENERATED_FILES
  ${generated_root}/graphics/materials_generated.h
)

add_library(graphics 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
  ${SHADER_FILES}
  ${SCHEMA_FILES}
  ${GENERATED_FILES}
)

settingsCR(graphics)
source_group(TREE ${assets_root}/Graphics FILES ${SHADER_FILES})
source_group(TREE ${root}/source FILES ${SCHEMA_FILES})

#microsoft bug see https://developercommunity.visualstudio.com/t/warning-C4005:-Outptr:-macro-redefinit/1546919
target_compile_options(graphics PRIVATE /WX-)

file(MAKE_DIRECTORY ${generated_root}/graphics)

add_custom_command(
    OUTPUT ${generated_root}/graphics/materials_generated.h
    COMMAND ${FLATC}
    ARGS --cpp --cpp-std C++17
    ARGS -o ${generated_root}/graphics/ ${root}/source/schemas/materials.fbs
    DEPENDS ${root}/source/schemas/materials.fbs
    VERBATIM
)

target_link_libraries(graphics PUBLIC
	headerUnits
    flatbuffers
    simdjson
    assets
	fmt
	glm
	core
	platform
	Vulkan::Vulkan volk
)

set_property(TARGET graphics APPEND PROPERTY FOLDER Engine)

target_compile_definitions(graphics PUBLIC SCHEMAS_MATERIALS="${root}/source/schemas/materials.fbs")
