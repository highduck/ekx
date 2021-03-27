cmake_minimum_required(VERSION 3.13)
project(plugin-admob)

add_library(${PROJECT_NAME} STATIC
        src/admob.hpp
        src/admob.cpp

        src/AdMobWrapper.hpp
        src/AdMobWrapper.cpp
        src/AdMobNull.cpp
        src/AdMobNull.cpp
        src/AdMobSimulator.hpp
        src/AdMobSimulator.cpp

        null/admob_null.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME}
        PUBLIC scenex
        )

#target_compile_options(${PROJECT_NAME} PUBLIC -std=c++17)
set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
