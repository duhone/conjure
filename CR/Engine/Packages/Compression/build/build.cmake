set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(INTERFACE_FILES
    ${root}/interface/Compression.ixx
    ${root}/interface/General.ixx
    ${root}/interface/Wave.ixx
)

set(SOURCE_FILES
    ${root}/source/General.cpp
)

set(BUILD_FILES
    ${root}/build/build.cmake
)

add_library(compression 
  ${INTERFACE_FILES} 
  ${SOURCE_FILES} 
  ${BUILD_FILES}
)

settingsCR(compression)

target_link_libraries(compression PUBLIC
	headerUnits
	core
	platform
	zstd
)

set_property(TARGET compression APPEND PROPERTY FOLDER Engine)
	
###############################################
#unit tests
###############################################
set(SOURCE_FILES
	${root}/tests/main.cpp
	${root}/tests/General.cpp
)

add_executable(compression_tests 
	${SOURCE_FILES}
)
		
settingsCR(compression_tests)
	
target_link_libraries(compression_tests 
	headerUnits
	doctest
	compression
	core
	platform
)		

set_property(TARGET compression_tests APPEND PROPERTY FOLDER Engine/Tests)

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

add_custom_command(TARGET compression_tests POST_BUILD        
COMMAND ${CMAKE_COMMAND} -E copy_if_different  
	${TEST_DATA}
	$<TARGET_FILE_DIR:compression_tests>)