set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Audio.ixx
    ${root}/interface/Service.ixx
    ${root}/interface/FX.ixx
    ${root}/interface/Music.ixx
)

set(SOURCE_FILES
    ${root}/source/AudioDevice.ixx
    ${root}/source/ChannelWeights.ixx
    ${root}/source/Constants.ixx
    ${root}/source/FXLibrary.ixx
    ${root}/source/MusicLibrary.ixx
    ${root}/source/OutputConversion.ixx
    ${root}/source/Sample.ixx
    ${root}/source/Utilities.ixx
    ${root}/source/Windows/AudioDevice.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

set(SCHEMA_FILES
    ${root}/source/schemas/music.fbs
    ${root}/source/schemas/soundfx.fbs
)

set(GENERATED_FILES
  ${generated_root}/audio/music_generated.h
  ${generated_root}/audio/soundfx_generated.h
)

add_library(audio 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
  ${SCHEMA_FILES}
  ${GENERATED_FILES}
)

settingsCR(audio)
source_group(TREE ${root}/source FILES ${SCHEMA_FILES})

#microsoft bug see https://developercommunity.visualstudio.com/t/warning-C4005:-Outptr:-macro-redefinit/1546919
target_compile_options(audio PRIVATE /WX-)

file(MAKE_DIRECTORY ${generated_root}/audio)

compileFlatbuffersSchema(audio music)
compileFlatbuffersSchema(audio soundfx)

target_link_libraries(audio PUBLIC
    assets
	compression
	headerUnits
	drlibs
    flatbuffers
	fmt
	function2
	glm
	opus
	samplerate
	core
	platform
	Rtworkq.lib
)

set_property(TARGET audio APPEND PROPERTY FOLDER Engine)

