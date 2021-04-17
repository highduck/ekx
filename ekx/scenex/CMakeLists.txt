cmake_minimum_required(VERSION 3.13)
project(scenex)

add_library(${PROJECT_NAME} STATIC "")

option(EK_UITEST "Enable features for UI testing" OFF)

if(EK_UITEST)
    message(STATUS "UI Testing enabled")
    add_definitions(-DEK_UITEST)
endif(EK_UITEST)

add_subdirectory(ek/scenex)
add_subdirectory(ek/goodies)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        #UNITY_BUILD ON
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ek
        PUBLIC sg-file)

if(EKX_BUILD_DEV_TOOLS)
    target_link_libraries(${PROJECT_NAME} PUBLIC ek-dev-tools)
    target_compile_definitions(${PROJECT_NAME} PUBLIC EK_DEV_TOOLS)
endif()

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
        -fstrict-aliasing
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
endif()