cmake_minimum_required(VERSION 3.19)
include(npm.cmake)
project(ekx)

option(EKX_BUILD_TESTS "Build EKX unit tests" OFF)
mark_as_advanced(EKX_BUILD_TESTS)

option(EKX_BUILD_COVERAGE "Build EKX coverage check" OFF)
option(EKX_BUILD_BENCHMARKS "Build EKX benchmarks projects" OFF)
option(EKX_BUILD_DEV_TOOLS "Build EKX in-game development tools" ON)

if ($ENV{CLION_IDE})
set(EKX_BUILD_TESTS ON)
set(EKX_BUILD_BENCHMARKS OFF)
endif ()

if (EKX_BUILD_TESTS)
    enable_testing()
endif ()

if (EKX_BUILD_TESTS AND BUILD_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # NDEBUG - ignore assertation coverage
    set(EK_COVERAGE_BUILD_COMPILE_FLAGS "-O0 -g -fno-inline -fprofile-arcs -ftest-coverage -fno-omit-frame-pointer -fno-optimize-sibling-calls -DNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EK_COVERAGE_BUILD_COMPILE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EK_COVERAGE_BUILD_COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

add_subdirectory(core)
add_subdirectory(app)
add_subdirectory(ek)
add_subdirectory(scenex)
add_subdirectory(sg-file)
add_subdirectory(physics)

add_library(ekx::ekx ALIAS scenex)

add_subdirectory(dev-tools)

if (EKX_BUILD_DEV_TOOLS)
    add_compile_definitions(EK_DEV_TOOLS)
    target_link_libraries(scenex
            PUBLIC ek-dev-tools
            )
endif ()

if (EKX_BUILD_BENCHMARKS)
    add_subdirectory(tests/ecxx/benchmarks)
endif ()