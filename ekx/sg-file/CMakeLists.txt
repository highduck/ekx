cmake_minimum_required(VERSION 3.19)
project(sg-file)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ./src)

target_link_libraries(${PROJECT_NAME} INTERFACE ek-core)