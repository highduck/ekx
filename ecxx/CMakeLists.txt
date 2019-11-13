cmake_minimum_required(VERSION 3.12)
project(ecxx LANGUAGES CXX)

add_subdirectory(src)

if (EKX_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()

if (EKX_BUILD_TESTS)
    add_subdirectory(test)
endif ()