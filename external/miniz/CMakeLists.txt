cmake_minimum_required(VERSION 3.13)
project(miniz)

add_library(${PROJECT_NAME} STATIC
        miniz/miniz.c
        miniz/miniz_tdef.c
        miniz/miniz_tdef.h
        miniz/miniz_tinfl.c
        miniz/miniz_tinfl.h
        miniz/miniz_zip.c
        miniz/miniz_zip.h
        miniz/zip_file.hpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)
