cmake_minimum_required(VERSION 3.13)
project(imgui)

add_library(${PROJECT_NAME} STATIC

        imgui_demo.cpp

        imconfig.h
        imgui.cpp
        imgui.h
        imgui_draw.cpp
        imgui_internal.h
        imgui_widgets.cpp
        imstb_textedit.h

        misc/cpp/imgui_stdlib.h
        misc/cpp/imgui_stdlib.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)
target_include_directories(${PROJECT_NAME} PUBLIC misc/cpp)
target_link_libraries(${PROJECT_NAME} stb)