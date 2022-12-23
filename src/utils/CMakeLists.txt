cmake_minimum_required(VERSION 3.8)

add_library(utils
    INTERFACE
)

add_library(safec::utils ALIAS utils)

target_include_directories(utils INTERFACE ${PROJECT_SOURCE_DIR})
