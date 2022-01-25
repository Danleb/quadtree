add_library(quadtree STATIC "")

file (GLOB COLLECTED_SOURCES "*.cpp")
target_sources(quadtree PRIVATE 
	${COLLECTED_SOURCES}
)

target_include_directories(quadtree PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../
)

CONAN_TARGET_LINK_LIBRARIES(quadtree)
