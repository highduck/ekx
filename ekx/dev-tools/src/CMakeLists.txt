cmake_minimum_required(VERSION 3.19)

add_library(${PROJECT_NAME} STATIC

        ek/editor/gui/EditorGUI.cpp
        ek/editor/gui/Widgets_impl.hpp

        ek/editor/imgui/ImGuiIntegration.hpp
        ek/editor/imgui/ImGuiIntegration.cpp
        ek/editor/imgui/imgui.hpp

        ek/editor/Editor.cpp
        ek/editor/Editor.hpp

        ImGuizmo/ImCurveEdit.cpp
        ImGuizmo/ImGradient.cpp
        ImGuizmo/ImGuizmo.cpp
        ImGuizmo/ImSequencer.cpp
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

if(EKX_BUILD_DEV_TOOLS)
target_compile_definitions(ek-core
        PUBLIC EK_ALLOCATION_TRACKER
        PUBLIC EK_ALLOCATION_TRACKER_STATS)
endif()


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
        PUBLIC ekx::dev-fonts
        PUBLIC scenex
        PUBLIC imgui
        PUBLIC pugixml
        )

#set_target_properties(${PROJECT_NAME} PROPERTIES
#        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build")