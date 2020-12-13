cmake_minimum_required(VERSION 3.13)
project(miniz)

add_library(${PROJECT_NAME} STATIC
        miniz/miniz.h
        miniz/miniz.c
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)
