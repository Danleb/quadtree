add_executable(quadtree_perf "")

file (GLOB COLLECTED_SOURCES "*.cpp")
target_sources(quadtree_perf PRIVATE 
	${COLLECTED_SOURCES}
)

target_include_directories(quadtree_perf PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

CONAN_TARGET_LINK_LIBRARIES(quadtree_perf)

target_link_libraries(quadtree_perf
	quadtree
)
