cmake_minimum_required(VERSION 3.8)

add_library(logger
    Logger.cpp
)

target_include_directories(logger
    PUBLIC
        ${PROJECT_SOURCE_DIR}
)

add_library(safec::logger ALIAS logger)
