cmake_minimum_required(VERSION 3.13)
project(mini-ads)

add_library(${PROJECT_NAME} STATIC
        src/ek/ext/mini_ads/mini_ads.hpp
        src/ek/ext/mini_ads/mini_ads.cpp
        null/mini_ads_null.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} ek)

#target_compile_options(${PROJECT_NAME} PUBLIC -std=c++17)
set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
