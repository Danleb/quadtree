add_executable(quadtree_test "")

file (GLOB COLLECTED_SOURCES "*.cpp")
target_sources(quadtree_test PRIVATE 
	${COLLECTED_SOURCES}
)

target_include_directories(quadtree_test PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

CONAN_TARGET_LINK_LIBRARIES(quadtree_test)

target_link_libraries(quadtree_test
	quadtree
)

enable_testing()
add_test(NAME quadtree_test
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
	COMMAND quadtree_test
)
