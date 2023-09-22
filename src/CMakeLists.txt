cmake_minimum_required(VERSION 3.8)
project(SafeC CXX C)

include(${CMAKE_SOURCE_DIR}/cmake/CompilerOptions.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Deps.cmake)

add_subdirectory(logger)
add_subdirectory(parser)
add_subdirectory(generator)
add_subdirectory(utils)
add_subdirectory(config)

add_executable(SafeCTranspiler
    main.cpp
)

target_link_libraries(SafeCTranspiler
    PUBLIC
        CONAN_PKG::boost
        safec::parser
        safec::generator
        safec::config
)


# make sure IDE indexes also the test files
file(GLOB SAFEC_TEST_FILES ${PROJECT_SOURCE_DIR}/safec_testfiles/*.sc)
add_custom_target(SafeCTestSources
    SOURCES
        ${SAFEC_TEST_FILES}
)
