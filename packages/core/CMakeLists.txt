cmake_minimum_required(VERSION 3.19)
add_subdirectory(src)

if (EKX_BUILD_TESTS)
    add_subdirectory(test)
endif ()