cmake_minimum_required(VERSION 3.12)

project(ekx LANGUAGES CXX)

option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_COVERAGE "Build coverage check" OFF)
option(BUILD_BENCHMARKS "Build benchmarks projects" OFF)

message(INFO "[${PROJECT_NAME}] loading `${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake`")
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
message(INFO "[${PROJECT_NAME}] =>> CONAN_LIBS = ${CONAN_LIBS}")

#if ($ENV{CLION_IDE})
#    set(BUILD_TESTS 1)
#    set(BUILD_BENCHMARKS 1)
#endif ()

if (BUILD_TESTS AND BUILD_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-inline -fprofile-arcs -ftest-coverage -fno-omit-frame-pointer -fno-optimize-sibling-calls")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    add_definitions(-DNDEBUG) # ignore assertation coverage
endif ()

if (BUILD_TESTS)
    enable_testing()
endif ()

add_subdirectory(ecxx)
add_subdirectory(core)
add_subdirectory(ek)
add_subdirectory(scenex)
add_subdirectory(flash)
add_subdirectory(editor)

#add_library(ekx INTERFACE)
#target_link_libraries(ekx INTERFACE scenex)