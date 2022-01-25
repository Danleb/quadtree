add_executable(quadtree_app "")

file (GLOB COLLECTED_SOURCES "*.cpp")
target_sources(quadtree_app PRIVATE 
	${COLLECTED_SOURCES}
)

target_include_directories(quadtree_app PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(quadtree_app
	quadtree
)

CONAN_TARGET_LINK_LIBRARIES(quadtree_app)
