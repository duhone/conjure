set(root "${CMAKE_CURRENT_LIST_DIR}/..")

find_package(Vulkan REQUIRED)

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Graphics.ixx
    ${root}/interface/Service.ixx
)

set(SOURCE_FILES
    ${root}/Source/DeviceService.ixx
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

target_compile_definitions(graphics PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_include_directories(graphics PRIVATE
	$ENV{VULKAN_SDK}/include
	Vulkan::Vulkan
)		

target_link_libraries(graphics PUBLIC
	headerUnits
	fmt
	glm
	spdlog
	core
	platform
	Vulkan::Vulkan 
)

set_property(TARGET graphics APPEND PROPERTY FOLDER Engine)

