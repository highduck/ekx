cmake_minimum_required(VERSION 3.12)
project(pugixml LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC
        pugixml.cpp
        pugixml.hpp
        pugiconfig.hpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)
