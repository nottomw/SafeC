cmake_minimum_required(VERSION 3.8)

add_library(walkers
    SemNodeWalker.cpp
    WalkerPrint.cpp
)

add_library(safec::walkers ALIAS walkers)

target_include_directories(walkers
    PRIVATE
        ${PROJECT_SOURCE_DIR}/parser
)
