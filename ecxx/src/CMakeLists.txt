cmake_minimum_required(VERSION 3.12)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE .)
target_compile_options(${PROJECT_NAME} INTERFACE -std=c++17)