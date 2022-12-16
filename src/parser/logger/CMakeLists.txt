cmake_minimum_required(VERSION 3.8)

add_library(logger
    Logger.cpp
)

add_library(safec::logger ALIAS logger)
