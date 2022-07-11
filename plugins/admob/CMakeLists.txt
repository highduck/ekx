cmake_minimum_required(VERSION 3.19)
project(plugin-admob)

add_library(${PROJECT_NAME} STATIC
        src/ek_admob.c
        src/AdMobWrapper.cpp
        src/AdMobSimulator.hpp
        src/AdMobNull.hpp
        )
add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
        PUBLIC plugin-firebase
        PUBLIC scenex
        )

target_compile_options(${PROJECT_NAME}
        PUBLIC -fno-exceptions
        PUBLIC -fno-rtti
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

    set_source_files_properties("src/ek_admob.c" PROPERTIES
            COMPILE_FLAGS "-x objective-c")

endif()