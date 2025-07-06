set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Graphics.ixx
    ${root}/interface/HandleTypes.ixx
    ${root}/interface/Service.ixx
    ${root}/interface/Sprites.ixx
)

set(SOURCE_FILES
    ${root}/source/Core.h
    ${root}/source/Constants.ixx
    ${root}/source/Context.ixx
    ${root}/source/DeviceService.ixx
    ${root}/source/CommandPool.ixx
    ${root}/source/Commands.ixx
    ${root}/source/ComputePipelines.ixx
    ${root}/source/Formats.ixx
    ${root}/source/GraphicsThread.ixx
    ${root}/source/InternalHandles.ixx
    ${root}/source/Materials.ixx
    ${root}/source/MultiDrawBuffer.ixx
    ${root}/source/Shaders.ixx
    ${root}/source/Sprites.cpp
    ${root}/source/SpritesInternal.ixx
    ${root}/source/Textures.ixx
    ${root}/source/UniformBuffer.ixx
    ${root}/source/Utils.ixx
    ${root}/source/VertexLayout.ixx
    ${root}/source/VertexBuffers.ixx
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
    ${root}/source/schemas/computePipelines.fbs
    ${root}/source/schemas/materials.fbs
    ${root}/source/schemas/shaders.fbs
    ${root}/source/schemas/sprites.fbs
    ${root}/source/schemas/textures.fbs
)

set(GENERATED_FILES
  ${generated_root}/graphics/computePipelines_generated.h
  ${generated_root}/graphics/materials_generated.h
  ${generated_root}/graphics/shaders_generated.h
  ${generated_root}/graphics/sprites_generated.h
  ${generated_root}/graphics/textures_generated.h
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

compileFlatbuffersSchema(graphics materials)
compileFlatbuffersSchema(graphics computePipelines)
compileFlatbuffersSchema(graphics shaders)
compileFlatbuffersSchema(graphics sprites)
compileFlatbuffersSchema(graphics textures)

target_link_libraries(graphics PUBLIC
	headerUnits
    flatbuffers
    assets
	fmt
	glm
	core
	platform
	volk
    vulkan_utilities
    vulkan_vma
    libjxl
    highway
    brotli
)

set_property(TARGET graphics APPEND PROPERTY FOLDER Engine)
