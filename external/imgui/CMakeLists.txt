cmake_minimum_required(VERSION 3.19)
project(imgui)

add_library(${PROJECT_NAME} STATIC
        src/imconfig.h
        src/imgui_demo.cpp
        src/imgui.cpp
        src/imgui.h
        src/imgui_draw.cpp
        src/imgui_internal.h
        src/imgui_widgets.cpp
        src/imgui_tables.cpp
        src/imstb_textedit.h
        src/imgui_stdlib.h
        src/imgui_stdlib.cpp
        src/misc/freetype/imgui_freetype.cpp
        src/misc/freetype/imgui_freetype.h

        src/implot.cpp
        src/implot_items.cpp
        src/implot_demo.cpp

        src/ImGuizmo/GraphEditor.cpp
        src/ImGuizmo/ImCurveEdit.cpp
        src/ImGuizmo/ImGradient.cpp
        src/ImGuizmo/ImGuizmo.cpp
        src/ImGuizmo/ImSequencer.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC ./src)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::headers
        PUBLIC freetype
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (EKX_BUILD_EXTERNAL_TESTS)
    add_subdirectory(test)
endif ()
