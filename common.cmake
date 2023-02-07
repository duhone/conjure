set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Do not build in-source.")
endif()

# WORKAROUND
# This is added so that CMake can recognize the .ixx extension as a module interface.
set(CMAKE_CXX_SYSROOT_FLAG_CODE "list(APPEND CMAKE_CXX_SOURCE_FILE_EXTENSIONS ixx)")

# set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=cppcoreguidelines-*)

function(addCommon target)		
	# using rtti for service locator
	#target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/GR->)
	
	target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX>)
	target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/fp:fast>)
	target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/fp:except->)
	target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/MP>)

	target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:Clang>:-ffast-math>)

	target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-ffast-math>)
		
	target_compile_options(${target} PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Profile>,$<CONFIG:Final>>>:/O2>)
	target_compile_options(${target} PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Profile>,$<CONFIG:Final>>>:/Oi>)
	target_compile_options(${target} PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Profile>,$<CONFIG:Final>>>:/Ot>)
	target_compile_options(${target} PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Profile>,$<CONFIG:Final>>>:/Ob2>)
	target_compile_options(${target} PRIVATE /sdl-)
		
	target_compile_definitions(${target} PRIVATE $<$<CONFIG:Debug>:CR_DEBUG=1>)
	target_compile_definitions(${target} PRIVATE $<$<NOT:$<CONFIG:Debug>>:CR_DEBUG=0>)
	target_compile_definitions(${target} PRIVATE $<$<CONFIG:RelWithDebInfo>:CR_RELEASE=1>)
	target_compile_definitions(${target} PRIVATE $<$<NOT:$<CONFIG:RelWithDebInfo>>:CR_RELEASE=0>)
	target_compile_definitions(${target} PRIVATE $<$<CONFIG:Profile>:CR_PROFILE=1>)
	target_compile_definitions(${target} PRIVATE $<$<NOT:$<CONFIG:Profile>>:CR_PROFILE=0>)
	target_compile_definitions(${target} PRIVATE $<$<CONFIG:Final>:CR_FINAL=1>)
	target_compile_definitions(${target} PRIVATE $<$<NOT:$<CONFIG:Final>>:CR_FINAL=0>)
	target_compile_definitions(${target} PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:Profile>,$<CONFIG:Final>>>:NDEBUG>)
	
	# generator expressions aren't working with INTERPROCEDURAL_OPTIMIZATION_PROFILE
	set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_PROFILE TRUE)
	set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_FINAL TRUE)	
	
	target_link_options(${target} PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>:/Debug:fastlink>)		
endfunction()


function(settings3rdParty target)	
	addCommon(${target})
	
	source_group("Interface" FILES ${INTERFACE_FILES})
	source_group("Source" FILES ${SOURCE_FILES})
	source_group("Build" FILES ${BUILD_FILES})
	
	target_compile_options(${target} PRIVATE /W0)
	target_compile_options(${target} PRIVATE /WX-)
	
	set_property(TARGET ${target} APPEND PROPERTY FOLDER 3rdParty)
endfunction()

function(settingsCR target)	
	addCommon(${target})
	
	source_group(TREE ${root} FILES ${INTERFACE_FILES})
	source_group(TREE ${root} FILES ${SOURCE_FILES})
	source_group(TREE ${root} FILES ${BUILD_FILES})
		
	target_compile_options(${target} PRIVATE /W4)
	target_compile_options(${target} PRIVATE /WX)
	
	# Version can be packed into a single 32 bit int
	# the max major version is 31. the max minor version is 255. patch can be up to 512K
	target_compile_definitions(${target} PRIVATE CR_VERSION_ENGINE_MAJOR=0)
	target_compile_definitions(${target} PRIVATE CR_VERSION_ENGINE_MINOR=1)
	target_compile_definitions(${target} PRIVATE CR_VERSION_ENGINE_PATCH=0)
	math(EXPR engineVersion "(0<<27)|(1<<19) | 0" OUTPUT_FORMAT HEXADECIMAL)
	target_compile_definitions(${target} PRIVATE CR_VERSION_ENGINE=${engineVersion})
	target_compile_definitions(${target} PRIVATE CR_VERSION_ENGINE_STRING="0.1.0")
	target_compile_definitions(${target} PRIVATE CR_ENGINE_NAME="Conjure")
	target_compile_definitions(${target} PRIVATE CR_VERSION_APP_MAJOR=0)
	target_compile_definitions(${target} PRIVATE CR_VERSION_APP_MINOR=1)
	target_compile_definitions(${target} PRIVATE CR_VERSION_APP_PATCH=0)
	math(EXPR appVersion "(0<<27)|(1<<19) | 0" OUTPUT_FORMAT HEXADECIMAL)
	target_compile_definitions(${target} PRIVATE CR_VERSION_APP=${appVersion})
	target_compile_definitions(${target} PRIVATE CR_VERSION_APP_STRING="0.1.0")
	target_compile_definitions(${target} PRIVATE CR_APP_NAME="Conjure")
	
	# disable unit tests in profile and final builds
	# target_compile_definitions(${target} PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:Profile>,$<CONFIG:Final>>>:DOCTEST_CONFIG_DISABLE>)
	
	#set_target_properties(${target} PROPERTIES
		#VS_GLOBAL_RunCodeAnalysis false

		# Use visual studio core guidelines
		# VS_GLOBAL_EnableMicrosoftCodeAnalysis false

		# Use clangtidy
		# VS_GLOBAL_EnableClangTidyCodeAnalysis true
		# VS_GLOBAL_ClangTidyChecks "-checks=-*,modernize-*, -modernize-avoid-c-arrays, -modernize-use-trailing-return-type, \
#bugprone-*, -bugprone-bool-pointer-implicit-conversion, cppcoreguidelines-*, -cppcoreguidelines-avoid-c-arrays, -cppcoreguidelines-pro-bounds-constant-array-index, misc-*, performance-*, readability-*, -readability-uppercase-literal-suffix"
	#)
endfunction()
