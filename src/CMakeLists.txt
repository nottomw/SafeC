cmake_minimum_required(VERSION 2.8)

project(SafeC CXX)
include(CompilerOptions.cmake)

add_executable(SafeCTranspiler
	main.cpp
)
