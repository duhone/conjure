set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Audio.ixx
    ${root}/interface/AudioEngine.ixx
    ${root}/interface/FX.ixx
    ${root}/interface/Mixer.ixx
    ${root}/interface/Tone.ixx
)

set(SOURCE_FILES
    ${root}/source/AudioDevice.ixx
    ${root}/source/AudioEngine.cpp
    ${root}/source/ChannelWeights.ixx
    ${root}/source/Constants.ixx
    ${root}/source/FXLibrary.ixx
    ${root}/source/Mixer.cpp
    ${root}/source/MixerSystem.ixx
    ${root}/source/MusicLibrary.ixx
    ${root}/source/OutputConversion.ixx
    ${root}/source/Sample.ixx
    ${root}/source/Services.ixx
    ${root}/source/Tone.cpp
    ${root}/source/ToneSystem.ixx
    ${root}/source/Utilities.ixx
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
	compression
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

