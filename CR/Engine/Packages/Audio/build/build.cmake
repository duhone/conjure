block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
)

set(CR_INTERFACE_MODULES
    ${root}/interface/Audio.ixx
    ${root}/interface/Handles.ixx
    ${root}/interface/FX.ixx
    ${root}/interface/Music.ixx
)

set(CR_IMPLEMENTATION
    ${root}/source/FXLibrary.ixx
    ${root}/source/MusicLibrary.ixx
    ${root}/source/Utilities.ixx
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

set(CR_SCHEMA_FILES
    ${root}/source/schemas/music.fbs
    ${root}/source/schemas/soundfx.fbs
)

set(CR_GENERATED_FILES
  ${generated_root}/generated/audio/music_generated.h
  ${generated_root}/generated/audio/soundfx_generated.h
)

add_library(audio)
settingsCR(audio)

#microsoft bug see https://developercommunity.visualstudio.com/t/warning-C4005:-Outptr:-macro-redefinit/1546919
target_compile_options(audio PRIVATE /WX-)

file(MAKE_DIRECTORY ${generated_root}/audio)

compileFlatbuffersSchema(audio music)
compileFlatbuffersSchema(audio soundfx)

target_link_libraries(audio PUBLIC
    assets
	compression
	drlibs
    flatbuffers
	glm
	opus
	core
	platform
    miniaudio
)

set_property(TARGET audio APPEND PROPERTY FOLDER Engine/Packages)

endblock()