cmake_minimum_required(VERSION 3.13)

project(stb-image-write)
add_library(${PROJECT_NAME} STATIC
        stb_image_write.h
        stb_image_write.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)
