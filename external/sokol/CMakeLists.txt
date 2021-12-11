cmake_minimum_required(VERSION 3.19)
project(sokol)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ./src)

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
