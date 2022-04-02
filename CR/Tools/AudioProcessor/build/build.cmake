set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(INTERFACE_FILES
)

set(SOURCE_FILES
    ${root}/source/main.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

set(TEST_FILES
    ${root}/test_assets/BGM_Menu.wav
)

add_executable(AudioProcessor   
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
	${TEST_FILES}
)

settingsCR(AudioProcessor)
			
target_link_libraries(AudioProcessor 
	cli11
	fmt
	opus
	spdlog
	core
	platform
)

source_group("Test Files" FILES ${TEST_FILES})
	
set_property(TARGET AudioProcessor APPEND PROPERTY FOLDER Tools)

add_custom_command(TARGET AudioProcessor POST_BUILD       
  COMMAND ${CMAKE_COMMAND} -E copy_if_different  
      ${TEST_FILES}
      $<TARGET_FILE_DIR:AudioProcessor>) 
		