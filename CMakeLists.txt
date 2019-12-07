cmake_minimum_required(VERSION 3.12)
project(ekx LANGUAGES CXX)

set(EKX_BUILD_TESTS OFF CACHE BOOL "Build EKX unit tests" FORCE)
mark_as_advanced(EKX_BUILD_TESTS)

set(EKX_BUILD_TESTS ON)

#if ($ENV{CLION_IDE})
#    set(EKX_BUILD_TESTS ON)
#endif ()

option(EKX_BUILD_COVERAGE "Build EKX coverage check" OFF)
option(EKX_BUILD_BENCHMARKS "Build EKX benchmarks projects" OFF)
option(EKX_BUILD_EDITOR "Build EKX editor projects" ON)

# Windows is not ready for Edit-mode
if (${CMAKE_SYSTEM_NAME} MATCHES Windows)
    set(EKX_BUILD_EDITOR OFF)
endif ()

# Mobile platforms are not capable to use Edit-Mode features
if ((${CMAKE_SYSTEM_NAME} MATCHES Emscripten) OR (${CMAKE_SYSTEM_NAME} MATCHES iOS) OR (${CMAKE_SYSTEM_NAME} MATCHES Android))
    set(EKX_MOBILE ON)
    set(EKX_DESKTOP OFF)
else ()
    set(EKX_MOBILE OFF)
    set(EKX_DESKTOP ON)
endif ()

if (EKX_MOBILE)
    set(EKX_BUILD_TESTS OFF)
    set(EKX_BUILD_COVERAGE OFF)
    set(EKX_BUILD_BENCHMARKS OFF)
    set(EKX_BUILD_EDITOR OFF)
endif ()

add_subdirectory(external)

if (EKX_BUILD_TESTS)
    enable_testing()
endif ()

if (EKX_BUILD_TESTS AND BUILD_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-inline -fprofile-arcs -ftest-coverage -fno-omit-frame-pointer -fno-optimize-sibling-calls")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    add_definitions(-DNDEBUG) # ignore assertation coverage
endif ()

add_subdirectory(ecxx)
add_subdirectory(core)
add_subdirectory(ek)
add_subdirectory(scenex)

add_subdirectory(extensions/mini-ads)

add_library(ekx INTERFACE)
target_link_libraries(ekx INTERFACE scenex ecxx ek ek-core)

if (EKX_BUILD_EDITOR)
    add_compile_definitions(EK_EDITOR)
    add_subdirectory(flash)
    add_subdirectory(editor)
    target_link_libraries(ekx INTERFACE ek-flash ek-editor)
endif ()

# Samples

add_subdirectory(samples/picos)