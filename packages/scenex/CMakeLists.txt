cmake_minimum_required(VERSION 3.19)
include(npm.cmake)
project(scenex)

add_library(${PROJECT_NAME} STATIC "")

add_library(ekx::scenex ALIAS scenex)
add_library(ekx::ekx ALIAS scenex)

option(EK_UITEST "Enable features for UI testing" OFF)

if(EK_UITEST)
    message(STATUS "UI Testing enabled")
    add_definitions(-DEK_UITEST)
endif(EK_UITEST)

add_subdirectory(src/ek/scenex)
add_subdirectory(src/ek/goodies)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        #UNITY_BUILD ON
        )

target_include_directories(${PROJECT_NAME} PUBLIC src)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::graphics
        PUBLIC ekx::audio
        PUBLIC ekx::local-storage
        PUBLIC ekx::texture-loader
        PUBLIC ekx::sg-file
        PUBLIC ekx::res
        PUBLIC ekx::ecs
        )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wno-dollar-in-identifier-extension
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types)

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
endif ()

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
endif()