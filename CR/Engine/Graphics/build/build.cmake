set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Graphics.ixx
    ${root}/interface/Service.ixx
)

set(SOURCE_FILES
    ${root}/Source/Core.h
    ${root}/Source/Constants.ixx
    ${root}/Source/Context.ixx
    ${root}/Source/DeviceService.ixx
    ${root}/Source/CommandPool.ixx
    ${root}/Source/Commands.ixx
    ${root}/Source/Materials.ixx
    ${root}/Source/Shaders.ixx
    ${root}/Source/Utils.ixx
    ${root}/Source/VkMemAllocator.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

set(SHADER_FILES
    ${assets_root}/Graphics/Shaders/sprite.comp
    ${assets_root}/Graphics/Shaders/sprite.vert
    ${assets_root}/Graphics/Shaders/sprite.frag
)

add_library(graphics 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
  ${SHADER_FILES}
)

settingsCR(graphics)
source_group(TREE ${assets_root}/Graphics FILES ${SHADER_FILES})

#microsoft bug see https://developercommunity.visualstudio.com/t/warning-C4005:-Outptr:-macro-redefinit/1546919
target_compile_options(graphics PRIVATE /WX-)

target_link_libraries(graphics PUBLIC
	headerUnits
    assets
	fmt
	glm
	core
	platform
	Vulkan::Vulkan volk
)

set_property(TARGET graphics APPEND PROPERTY FOLDER Engine)

