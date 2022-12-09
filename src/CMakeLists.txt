cmake_minimum_required(VERSION 3.8)

project(SafeC CXX)

include(${CMAKE_SOURCE_DIR}/cmake/CompilerOptions.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Deps.cmake)

add_executable(SafeCTranspiler
	main.cpp
)

target_link_libraries(SafeCTranspiler
	CONAN_PKG::boost
)
