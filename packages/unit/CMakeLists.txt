cmake_minimum_required(VERSION 3.19)

# STANDALONE project
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(UNIT_LIBRARY_PROJECT ON)
endif ()

project(unit C)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

# alias for NPM library name
add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (UNIT_LIBRARY_PROJECT)
    add_subdirectory(src)
endif ()

if (UNIT_LIBRARY_PROJECT)
    enable_testing()

    include(cmake/code-coverage.cmake)
    target_code_coverage(unit INTERFACE)

    add_subdirectory(test)
    add_subdirectory(example)

    coverage_all()
endif ()
