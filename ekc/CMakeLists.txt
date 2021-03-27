cmake_minimum_required(VERSION 3.13)
include(npm.cmake)
project(ekc)

add_executable(${PROJECT_NAME}
        src/cli_main.cpp
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g -Os)
endif ()

target_link_libraries(${PROJECT_NAME}
        PUBLIC ek-dev-tools
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(EKC_OS_PREFIX osx)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -Oz -flto)
        target_link_options(${PROJECT_NAME} PRIVATE -Wl,-dead_strip -Wl,-dead_strip_dylibs)
    endif ()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(EKC_OS_PREFIX win32)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(EKC_OS_PREFIX linux)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -Oz)
    endif ()
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${EKC_OS_PREFIX}"
        RUNTIME_OUTPUT_NAME "ekc"
        RUNTIME_OUTPUT_NAME_DEBUG "ekc-debug"
        )