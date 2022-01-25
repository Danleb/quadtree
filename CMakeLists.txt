cmake_minimum_required(VERSION 3.15)
project(Quadtree VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#if (MSVC)
	#message(${CMAKE_CXX_COMPILER_VERSION})
#endif()

include (utils.cmake)
InstallAndSetupConanPackages()

add_subdirectory(perf)
add_subdirectory(src)
add_subdirectory(test)

enable_testing()
