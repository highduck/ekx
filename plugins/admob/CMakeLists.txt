cmake_minimum_required(VERSION 3.19)
include(npm.cmake)
project(plugin-admob)

add_library(${PROJECT_NAME} STATIC
        src/admob.hpp
        src/admob.cpp
        src/admob_android.hpp
        src/admob_null.hpp
        src/admob_ios.hpp

        src/AdMobWrapper.hpp
        src/AdMobWrapper.cpp
        src/AdMobSimulator.hpp
        src/AdMobNull.hpp
        )
add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME}
        PUBLIC plugin-firebase
        PUBLIC scenex
        )

#target_compile_options(${PROJECT_NAME} PUBLIC -std=c++17)
set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    target_compile_options(${PROJECT_NAME}
            PUBLIC -x objective-c++
            )

endif()