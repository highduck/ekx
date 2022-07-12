cmake_minimum_required(VERSION 3.19)
project(quickjs C)

add_library(${PROJECT_NAME} STATIC
        src/cutils.c
        src/libbf.c
        src/libregexp.c
        src/libunicode.c
        src/quickjs.c
        src/quickjs-libc.c
        )

target_include_directories(${PROJECT_NAME} PUBLIC ./src)

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC ${CMAKE_DL_LIBS} m Threads::Threads)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC ${CMAKE_DL_LIBS} m Threads::Threads)
endif()

target_compile_definitions(${PROJECT_NAME}
        PUBLIC CONFIG_VERSION="1"
        PUBLIC _GNU_SOURCE
        )

if (EKX_BUILD_EXTERNAL_TESTS)
    add_subdirectory(test)
endif ()
