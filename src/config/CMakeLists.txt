cmake_minimum_required(VERSION 3.8)

add_library(config
    ConfigLex.cpp
)

add_library(safec::config ALIAS config)

target_include_directories(utils INTERFACE ${PROJECT_SOURCE_DIR})
