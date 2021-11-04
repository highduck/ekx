cmake_minimum_required(VERSION 3.19)
include(npm.cmake)
project(ecs)

add_library(${PROJECT_NAME} STATIC
        src/ecxx/impl/ecxx.cpp
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC src)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        C_STANDARD 17
        )

if (NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    set_source_files_properties(stb/stb_image.cpp PROPERTIES COMPILE_FLAGS -Wno-cast-align) #

    set(EK_COMPILE_WARNINGS
            -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wnull-dereference -Wpedantic -Wreturn-type
            -Woverloaded-virtual -Wcast-align -Wno-deprecated-declarations -Wstrict-aliasing)

    #set(EK_COMPILER_LTO "-flto")
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(EK_COMPILE_WARNINGS ${EK_COMPILE_WARNINGS}
            -Wno-dollar-in-identifier-extension -Wno-gnu-anonymous-struct -Wno-nested-anon-types)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    #set(EK_COMPILER_LTO "")
endif ()

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fstrict-aliasing
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC core
        )

if (EKX_BUILD_TESTS)
    add_subdirectory(test)
endif ()