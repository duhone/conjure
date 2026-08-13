block()

set(root "${CMAKE_CURRENT_LIST_DIR}/..")


set(CR_INTERFACE_HEADERS
)

set(CR_INTERFACE_MODULES
    ${root}/interface/Compression.ixx
    ${root}/interface/General.ixx
    ${root}/interface/Wave.ixx
)

set(CR_IMPLEMENTATION
    ${root}/source/General.cpp
)

set(CR_BUILD_FILES
    ${root}/build/build.cmake
)

set(CR_SCHEMA_FILES
)

set(CR_GENERATED_FILES
)

add_library(compression)
settingsCR(compression)

target_link_libraries(compression PUBLIC
	core
	platform
	zstd
)

set_property(TARGET compression APPEND PROPERTY FOLDER Engine/Packages)
	
###############################################
#unit tests
###############################################

set(CR_INTERFACE_HEADERS
)

set(CR_INTERFACE_MODULES
)

set(CR_IMPLEMENTATION
	${root}/tests/main.cpp
	${root}/tests/General.cpp
)

add_executable(compression_tests 
	${SOURCE_FILES}
)
		
settingsCR(compression_tests)
	
target_link_libraries(compression_tests 
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

endblock()