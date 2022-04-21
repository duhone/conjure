set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Audio.ixx
    ${root}/interface/AudioEngine.ixx
    ${root}/interface/MixerHandle.ixx
)

set(SOURCE_FILES
    ${root}/source/AudioDevice.ixx
    ${root}/source/AudioEngine.cpp
    ${root}/source/ChannelWeights.ixx
    ${root}/source/Constants.ixx
    ${root}/source/Mixer.ixx
    ${root}/source/MixerSystem.ixx
    ${root}/source/OutputConversion.ixx
    ${root}/source/Sample.ixx
    ${root}/source/TestTone.ixx
    ${root}/source/ToneSystem.ixx
    ${root}/source/Windows/AudioDevice.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(audio 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
)

settingsCR(audio)

target_link_libraries(audio PUBLIC
	headerUnits
	fmt
	function2
	glm
	opus
	samplerate
	spdlog
	core
	platform
	Rtworkq.lib
)

set_property(TARGET audio APPEND PROPERTY FOLDER Engine)

