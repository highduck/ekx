cmake_minimum_required(VERSION 3.13)

add_library(${PROJECT_NAME} STATIC

        ek/editor/gui/gui.hpp
        ek/editor/gui/inspector.cpp
        ek/editor/gui/hierarchy.cpp
        ek/editor/gui/stats.cpp
        ek/editor/gui/gui_editor.cpp
        ek/editor/gui/utils.cpp
        ek/editor/gui/gui_atlas.cpp
        ek/editor/gui/gui_sg_file.cpp
        ek/editor/gui/gui_font.cpp
        ek/editor/gui/gui_particles.cpp
        ek/editor/gui/resources_view.cpp

        ek/editor/imgui/imgui_module.hpp
        ek/editor/imgui/imgui_module.cpp
        ek/editor/imgui/imgui.hpp

        ek/editor/editor.cpp
        ek/editor/editor.hpp

        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)
target_compile_definitions(${PROJECT_NAME} PUBLIC
        EK_DEV_TOOLS
        )


if(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
target_compile_options(${PROJECT_NAME} PUBLIC
        -Wno-gnu-anonymous-struct
        -Wno-nested-anon-types
        #        -fno-exceptions
        )
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fstrict-aliasing
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC scenex
        PUBLIC imgui
        PUBLIC pugixml
        )

#set_target_properties(${PROJECT_NAME} PROPERTIES
#        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build")