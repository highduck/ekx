cmake_minimum_required(VERSION 3.19)
project(doctest)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ./doctest)

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
