cmake_minimum_required(VERSION 3.19)
project(sokol)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ./include)

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (EKX_BUILD_EXTERNAL_TESTS)
    add_subdirectory(test)
endif ()
