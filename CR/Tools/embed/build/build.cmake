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
    ${root}/test_assets/shader.crsm
)

add_executable(embed   
	${INTERFACE_FILES} 
	${SOURCE_FILES} 
	${BUILD_FILES}
	${TEST_FILES}
)

settingsCR(embed)
			
target_link_libraries(embed 
	cli11
	fmt
    glm
	spdlog
	core
	platform
)

source_group("Test Files" FILES ${TEST_FILES})
	
set_property(TARGET embed APPEND PROPERTY FOLDER Tools)

add_custom_command(TARGET embed POST_BUILD       
  COMMAND ${CMAKE_COMMAND} -E copy_if_different  
      ${TEST_FILES}
      $<TARGET_FILE_DIR:embed>) 
		