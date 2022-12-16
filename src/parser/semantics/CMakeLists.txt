cmake_minimum_required(VERSION 3.8)

add_subdirectory(walkers)

add_library(semantics
    Semantics.cpp
    SemNode.cpp
)

add_library(safec::semantics ALIAS semantics)

target_link_libraries(semantics
    PUBLIC
        safec::walkers
        safec::logger 
)

target_include_directories(semantics
    PRIVATE
        ${PROJECT_SOURCE_DIR}/parser
)
