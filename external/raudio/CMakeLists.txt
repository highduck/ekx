cmake_minimum_required(VERSION 3.13)
project(raudio)

add_library(${PROJECT_NAME} STATIC
        raudio.h
        raudio.cpp
        external/dr_mp3.h
        external/miniaudio.h
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)
target_compile_definitions(${PROJECT_NAME} PUBLIC
        RAUDIO_STANDALONE
        SUPPORT_FILEFORMAT_MP3
        )
