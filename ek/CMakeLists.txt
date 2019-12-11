cmake_minimum_required(VERSION 3.13)
project(ek LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC "")

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} ek-core)

target_compile_options(${PROJECT_NAME} PUBLIC -std=c++17)
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

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME}
            PRIVATE
            -fno-exceptions
            -fstrict-aliasing
            -g
            ${EK_COMPILE_WARNINGS}
            )
else ()
    if (${CMAKE_SYSTEM_NAME} MATCHES "Android" OR
            ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        target_compile_options(${PROJECT_NAME} PRIVATE
                -Os -fno-exceptions
                -fstrict-aliasing
                ${EK_COMPILE_WARNINGS})
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE
                -Oz -flto -fno-exceptions -fno-rtti -fstrict-aliasing ${EK_COMPILE_WARNINGS})
    endif ()
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_subdirectory(platforms/web)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    add_subdirectory(platforms/apple)
    add_subdirectory(platforms/ios)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_subdirectory(platforms/apple)
    add_subdirectory(platforms/mac)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    add_subdirectory(platforms/android)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_subdirectory(platforms/windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_subdirectory(platforms/linux)
else ()
    error("Platform: UNKNOWN")

endif ()

if (EKX_BUILD_TESTS)
    add_subdirectory(test)
endif ()