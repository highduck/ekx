cmake_minimum_required(VERSION 3.19)
include(npm.cmake)
project(plugin-firebase)

add_library(${PROJECT_NAME} STATIC
        src/ek/firebase.c
        )
add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC src)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::app
        )

#target_compile_options(${PROJECT_NAME} PUBLIC -std=c++17)
set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin|iOS")

    set_source_files_properties("src/ek/firebase.c" PROPERTIES
            COMPILE_FLAGS "-x objective-c")

endif()

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )