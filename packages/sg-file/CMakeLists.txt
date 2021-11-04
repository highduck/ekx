cmake_minimum_required(VERSION 3.19)
include(npm.cmake)
project(sg-file)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ./src)

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} INTERFACE ekx::core)