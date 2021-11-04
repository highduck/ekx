cmake_minimum_required(VERSION 3.19)
include(npm.cmake)
add_subdirectory(src)

if (EKX_BUILD_TESTS)
    add_subdirectory(test)
endif ()