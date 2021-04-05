cmake_minimum_required(VERSION 3.13)
include(npm.cmake)
project(ekx)

set(EKX_BUILD_TESTS OFF CACHE BOOL "Build EKX unit tests" FORCE)
mark_as_advanced(EKX_BUILD_TESTS)

option(EKX_BUILD_COVERAGE "Build EKX coverage check" OFF)
option(EKX_BUILD_BENCHMARKS "Build EKX benchmarks projects" OFF)
option(EKX_BUILD_DEV_TOOLS "Build EKX in-game development tools" ON)

if ($ENV{CLION_IDE})
set(EKX_BUILD_TESTS ON)
#set(EKX_BUILD_BENCHMARKS ON)
endif ()

if (EKX_BUILD_TESTS)
    enable_testing()
endif ()

if (EKX_BUILD_TESTS AND BUILD_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-inline -fprofile-arcs -ftest-coverage -fno-omit-frame-pointer -fno-optimize-sibling-calls")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    add_definitions(-DNDEBUG) # ignore assertation coverage
endif ()

add_subdirectory(core)
add_subdirectory(ek)
add_subdirectory(scenex)
add_subdirectory(physics)

add_library(ekx INTERFACE)
target_link_libraries(ekx
        INTERFACE ek-core
        INTERFACE ek
        INTERFACE scenex)

add_library(ekx::ekx ALIAS ekx)

add_subdirectory(flash)
add_subdirectory(dev-tools)

if (EKX_BUILD_DEV_TOOLS)
    add_compile_definitions(EK_DEV_TOOLS)
    target_link_libraries(ekx
            INTERFACE ek-dev-tools
            INTERFACE ek-flash)
endif ()

if (EKX_BUILD_TESTS)
    add_subdirectory(tests/core-test)
    add_subdirectory(tests/scenex-test)
endif ()

if (EKX_BUILD_BENCHMARKS)
    add_subdirectory(tests/ecxx/benchmarks)
endif ()