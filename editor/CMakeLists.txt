cmake_minimum_required(VERSION 3.13)

project(ek-editor)

add_subdirectory(src)

if (EKX_BUILD_TESTS)
    add_subdirectory(test)
endif ()

add_subdirectory(cli)