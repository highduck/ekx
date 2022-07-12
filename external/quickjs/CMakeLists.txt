cmake_minimum_required(VERSION 3.19)
project(quickjs)

add_library(${PROJECT_NAME} STATIC
        src/cutils.c
        src/libbf.c
        src/libregexp.c
        src/libunicode.c
        src/quickjs.c
        src/quickjs-libc.c
        )

target_include_directories(${PROJECT_NAME} PUBLIC ./src)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

if(UNIX OR MINGW)
    find_package(Threads)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} m Threads::Threads)
endif()

target_compile_definitions(${PROJECT_NAME}
        PUBLIC CONFIG_VERSION="1"
        PUBLIC _GNU_SOURCE
        )
