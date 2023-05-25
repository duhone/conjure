set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Graphics.ixx
    ${root}/interface/Service.ixx
)

set(SOURCE_FILES
    ${root}/Source/DeviceService.ixx
    ${root}/Source/Utils.ixx
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(graphics 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
)

settingsCR(graphics)
#microsoft bug see https://developercommunity.visualstudio.com/t/warning-C4005:-Outptr:-macro-redefinit/1546919
target_compile_options(graphics PRIVATE /WX-)

target_link_libraries(graphics PUBLIC
	headerUnits
	fmt
	glm
	core
	platform
	Vulkan::Vulkan volk
)

set_property(TARGET graphics APPEND PROPERTY FOLDER Engine)

