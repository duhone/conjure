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

set(CR_TEST_FILES
    ${root}/test_assets/shader.crsm
)

add_executable(embed)
settingsCR(embed)
			
target_link_libraries(embed PUBLIC
	cli11
	engine
)
	
set_property(TARGET embed APPEND PROPERTY FOLDER Tools)

add_custom_command(TARGET embed POST_BUILD       
  COMMAND ${CMAKE_COMMAND} -E copy_if_different  
      ${CR_TEST_FILES}
      $<TARGET_FILE_DIR:embed>) 
		
endblock()