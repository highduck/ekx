cmake_minimum_required(VERSION 3.19)
project(msgpack)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ./include)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})