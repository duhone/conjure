block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")

set(CR_INTERFACE_HEADERS
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
    ${root}/source/main.cpp
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

set(CR_SCHEMA_FILES
)

set(CR_GENERATED_FILES
)

set(CR_TEST_FILES
    ${root}/test_assets/BGM_Menu.wav
)

add_executable(AudioProcessor)
settingsCR(AudioProcessor)
			
target_link_libraries(AudioProcessor 
	cli11
	opus
	engine
)

set_property(TARGET AudioProcessor APPEND PROPERTY FOLDER Tools)

add_custom_command(TARGET AudioProcessor POST_BUILD       
  COMMAND ${CMAKE_COMMAND} -E copy_if_different  
      ${CR_TEST_FILES}
      $<TARGET_FILE_DIR:AudioProcessor>) 
		
endblock()