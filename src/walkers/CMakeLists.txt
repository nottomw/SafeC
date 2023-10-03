cmake_minimum_required(VERSION 3.8)

add_library(walkers
    SemNodeWalker.cpp
    WalkerPrint.cpp
    WalkerSourceCoverage.cpp
    WalkerSourceGen.cpp
    WalkerDeferExecute.cpp
)

add_library(safec::walkers ALIAS walkers)

target_include_directories(walkers
    PUBLIC
        ${PROJECT_SOURCE_DIR}/parser
)

# supressed because of peek() overloads - to strict
target_compile_options(walkers PRIVATE -Wno-unused-parameter)

target_link_libraries(walkers
    PRIVATE
        safec::logger
)
