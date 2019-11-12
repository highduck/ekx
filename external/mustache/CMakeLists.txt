cmake_minimum_required(VERSION 3.12)
project(mustache LANGUAGES CXX)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE .)

