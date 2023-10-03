cmake_minimum_required(VERSION 3.8)

add_library(semantics
    Semantics.cpp
)

add_library(safec::semantics ALIAS semantics)

target_link_libraries(semantics
    PUBLIC
        safec::walkers
        safec::logger
        safec::semantic_nodes
        CONAN_PKG::boost
)

target_include_directories(semantics
    PRIVATE
        ${PROJECT_SOURCE_DIR}/semantics
        ${PROJECT_SOURCE_DIR}/parser
)
