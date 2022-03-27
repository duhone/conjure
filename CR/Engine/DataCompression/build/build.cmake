set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/DataCompression.ixx
)

set(SOURCE_FILES
    ${root}/source/DataCompression.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(dataCompression 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
)

settingsCR(dataCompression)

target_link_libraries(dataCompression PUBLIC
	headerUnits
	core
	zstd
)

set_property(TARGET dataCompression APPEND PROPERTY FOLDER Engine)
	
###############################################
#unit tests
###############################################
set(SOURCE_FILES
	${root}/tests/main.cpp
	${root}/tests/DataCompression.cpp
)

add_executable(dataCompression_tests 
	${SOURCE_FILES}
)
		
settingsCR(dataCompression_tests)
	
target_link_libraries(dataCompression_tests 
	headerUnits
	doctest
	dataCompression
	core
	platform
)		

set_property(TARGET dataCompression_tests APPEND PROPERTY FOLDER Engine/Tests)

set(TEST_DATA 
	"${root}/tests/content/alice29.txt"
	"${root}/tests/content/bumble_tales.tga" 
	"${root}/tests/content/cp.html" 
	"${root}/tests/content/CS-3C1.tga"
	"${root}/tests/content/data.xml" 
	"${root}/tests/content/Game.cpp"
	"${root}/tests/content/kodim23.tga"
	"${root}/tests/content/lena3.tga"
	"${root}/tests/content/TitleThemeRemix.wav"
)

add_custom_command(TARGET dataCompression_tests POST_BUILD        
COMMAND ${CMAKE_COMMAND} -E copy_if_different  
	${TEST_DATA}
	$<TARGET_FILE_DIR:dataCompression_tests>)