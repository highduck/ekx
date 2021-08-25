cmake_minimum_required(VERSION 3.19)

project(native-lib)

add_library(${PROJECT_NAME} SHARED
        stub/stub.cpp #-SOURCES-#
        )

target_include_directories(${PROJECT_NAME} PUBLIC
        stub #-SEARCH_ROOTS-#
        )

target_link_libraries(${PROJECT_NAME} log android)