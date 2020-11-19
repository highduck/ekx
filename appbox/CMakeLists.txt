cmake_minimum_required(VERSION 3.13)
project(appbox)

add_library(${PROJECT_NAME} STATIC ""
        )

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME}
        scenex
        plugin-admob
        plugin-billing
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

if (NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    set(EK_COMPILE_WARNINGS
            -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wnull-dereference -Wpedantic -Wreturn-type -Woverloaded-virtual -Wcast-align
            -Wno-deprecated-declarations
            -DGL_SILENCE_DEPRECATION
            -Wstrict-aliasing)

    set(EK_COMPILER_LTO "-flto")
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(EK_COMPILE_WARNINGS ${EK_COMPILE_WARNINGS}
            -Wno-dollar-in-identifier-extension
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types
            )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(EK_COMPILER_LTO "")
endif ()

#target_compile_definitions(${PROJECT_NAME} PUBLIC)

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fstrict-aliasing
        ${EK_COMPILE_WARNINGS}
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Oz)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Android" OR
            ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -flto)
    endif ()
endif ()