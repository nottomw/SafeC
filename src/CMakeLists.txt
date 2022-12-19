cmake_minimum_required(VERSION 3.8)
project(SafeC CXX C)

include(${CMAKE_SOURCE_DIR}/cmake/CompilerOptions.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Deps.cmake)

add_subdirectory(logger)
add_subdirectory(parser)

add_executable(SafeCTranspiler
    main.cpp
)

target_link_libraries(SafeCTranspiler
    PUBLIC
        CONAN_PKG::boost
        safec::parser
)
