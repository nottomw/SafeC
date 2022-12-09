cmake_minimum_required(VERSION 2.8)

project(SafeC CXX)

# TODO: move project up
include(${CMAKE_SOURCE_DIR}/../cmake/CompilerOptions.cmake)

set(CMAKE_VERBOSE_MAKEFILE 1)

# TODO: move to cmake script
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES boost/1.80.0
                BUILD missing
                BASIC_SETUP CMAKE_TARGETS)

add_executable(SafeCTranspiler
	main.cpp
)

target_link_libraries(SafeCTranspiler
	CONAN_PKG::boost
)
