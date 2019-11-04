cmake_minimum_required(VERSION 3.12)

project(ek-editor LANGUAGES CXX)

add_subdirectory(src)

if (BUILD_TESTS)
    add_subdirectory(test)
endif ()

add_subdirectory(cli)