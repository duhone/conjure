set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
#set(CMAKE_CXX_MODULE_STD ON) //doesnt work with VS, but also doesn't seem to be needed

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Do not build in-source.")
endif()

set(conjure_root "${CMAKE_CURRENT_LIST_DIR}")
set(assets_root "${CMAKE_CURRENT_LIST_DIR}/CR/Assets")
set(generated_include "${CMAKE_BINARY_DIR}/generated")
set(generated_root "${CMAKE_BINARY_DIR}/generated/generated")

# set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=cppcoreguidelines-*)

function(addCommon target)
	# using rtti for service locator, not using service locator though, so disable.
	target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/GR->)
	# should also disable exceptions. cmake doesn't support that directly

	# secure versions don't seem available in std module
	target_compile_definitions(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
	
	target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)	

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
	
	target_include_directories(${target} SYSTEM PUBLIC "${generated_include}")

	target_link_options(${target} PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>:/Debug:fastlink>)		
endfunction()


function(settings3rdParty target)	
	addCommon(${target})
	
	target_sources(${target} PRIVATE ${CR_IMPLEMENTATION})
	target_sources(${target} PUBLIC FILE_SET HEADERS FILES ${CR_INTERFACE_HEADERS})
	target_sources(${target} PUBLIC FILE_SET CXX_MODULES FILES ${CR_INTERFACE_MODULES})
	target_sources(${target} PRIVATE ${CR_BUILD_FILES})
	target_sources(${target} PRIVATE ${CR_TEST_FILES})

	source_group(TREE ${root} FILES ${CR_INTERFACE_HEADERS})
	source_group(TREE ${root} FILES ${CR_INTERFACE_MODULES})
	source_group(TREE ${root} FILES ${CR_IMPLEMENTATION})
	source_group(TREE ${root} FILES ${CR_BUILD_FILES})
	source_group(TREE ${root} FILES ${CR_TEST_FILES})
	
	target_compile_options(${target} PRIVATE /W0)
	target_compile_options(${target} PRIVATE /WX-)
	
	set_property(TARGET ${target} APPEND PROPERTY FOLDER 3rdParty)
endfunction()

function(settingsCR target)	
	addCommon(${target})
	
	target_sources(${target} PRIVATE ${CR_IMPLEMENTATION})
	target_sources(${target} PUBLIC FILE_SET HEADERS FILES ${CR_INTERFACE_HEADERS})
	target_sources(${target} PUBLIC FILE_SET CXX_MODULES FILES ${CR_INTERFACE_MODULES})
	target_sources(${target} PRIVATE ${CR_BUILD_FILES})

	source_group(TREE ${root} FILES ${CR_INTERFACE_HEADERS})
	source_group(TREE ${root} FILES ${CR_INTERFACE_MODULES})
	source_group(TREE ${root} FILES ${CR_IMPLEMENTATION})
	source_group(TREE ${root} FILES ${CR_BUILD_FILES})
		
	target_compile_options(${target} PRIVATE /W4)
	target_compile_options(${target} PRIVATE /WX)
	target_compile_options(${target} PRIVATE /wd4324)
	# because of fmt, it spams this warning
	target_compile_options(${target} PRIVATE /wd4702)
	
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


function(compileFlatbuffersSchema target file)
	add_custom_command(
		OUTPUT ${generated_root}/${target}/${file}_generated.h
		COMMAND ${FLATC}
		ARGS --cpp --cpp-std C++17
		ARGS -o ${generated_root}/${target}/ ${root}/source/schemas/${file}.fbs
		DEPENDS ${root}/source/schemas/${file}.fbs
		VERBATIM
	)
	string(TOUPPER ${file} fileUpper)
	target_compile_definitions(${target} PUBLIC SCHEMAS_${fileUpper}="${root}/source/schemas/${file}.fbs")
endfunction()