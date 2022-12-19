cmake_minimum_required(VERSION 3.8)

add_subdirectory(semantics)
add_subdirectory(parser_generator)

add_library(parser
    Parser.cpp
)

add_library(safec::parser ALIAS parser)

target_link_libraries(parser
    PUBLIC
        CONAN_PKG::boost
    PRIVATE
        safec::logger
        safec::semantics
        safec::parser_generated
)
