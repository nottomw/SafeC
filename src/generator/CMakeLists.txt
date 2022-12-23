cmake_minimum_required(VERSION 3.8)

add_library(generator
    Generator.cpp
)

add_library(safec::generator ALIAS generator)

target_link_libraries(generator
    PUBLIC
        CONAN_PKG::boost
    PRIVATE
        safec::logger
        safec::parser
)
