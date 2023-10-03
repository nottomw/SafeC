cmake_minimum_required(VERSION 3.8)

add_library(semantic_nodes
    SemNode.cpp
)

add_library(safec::semantic_nodes ALIAS semantic_nodes)

target_include_directories(semantic_nodes
    PUBLIC
        ${PROJECT_SOURCE_DIR}/semantic_nodes
)

target_link_libraries(semantic_nodes
    PRIVATE
        safec::logger
)
