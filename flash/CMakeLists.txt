cmake_minimum_required(VERSION 3.13)
project(ek-flash)

add_library(${PROJECT_NAME}
        # flash document
        ek/flash/doc/flash_archive.cpp
        ek/flash/doc/flash_archive.hpp
        ek/flash/doc/flash_doc.cpp
        ek/flash/doc/flash_doc.hpp
        ek/flash/doc/types.hpp

        ek/flash/doc/parsing/basic_types.cpp
        ek/flash/doc/parsing/bitmap.cpp
        ek/flash/doc/parsing/edges.cpp
        ek/flash/doc/parsing/elements_parse.cpp
        ek/flash/doc/parsing/filters.cpp
        ek/flash/doc/parsing/graphics_parse.cpp
        ek/flash/doc/parsing/parse_blend_mode.cpp
        ek/flash/doc/parsing/parsing.hpp

        # rasterizer
        ek/flash/rasterizer/cairo_renderer.cpp
        ek/flash/rasterizer/cairo_renderer.hpp
        ek/flash/rasterizer/cairo_utility.cpp
        ek/flash/rasterizer/cairo_utility.hpp

        ek/flash/rasterizer/render_to_sprite.cpp
        ek/flash/rasterizer/render_to_sprite.hpp
        ek/flash/rasterizer/render_command.hpp
        ek/flash/rasterizer/dom_scanner.cpp
        ek/flash/rasterizer/dom_scanner.hpp
        ek/flash/rasterizer/shape_edge_decoder.cpp
        ek/flash/rasterizer/shape_edge_decoder.hpp
        ek/flash/rasterizer/shape_edge.cpp
        ek/flash/rasterizer/shape_edge.hpp
        ek/flash/rasterizer/shape_processor.cpp
        ek/flash/rasterizer/shape_processor.hpp
        ek/flash/rasterizer/transform_model.hpp
        ek/flash/rasterizer/software_filters.cpp
        ek/flash/rasterizer/software_filters.hpp

        ek/spritepack/sprite_data.hpp
        ek/spritepack/export_atlas.cpp
        ek/spritepack/export_atlas.hpp
        ek/spritepack/sprite_packing.cpp
        ek/spritepack/sprite_packing.hpp
        ek/spritepack/save_image_png.cpp
        ek/spritepack/save_image_png.hpp
        ek/spritepack/atlas_declaration.cpp
        ek/spritepack/atlas_declaration.hpp

        ek/fonts/export_font.cpp
        ek/fonts/export_font.hpp
        ek/fonts/ft_library.cpp
        ek/fonts/ft_library.hpp
        ek/fonts/font_declaration.cpp
        ek/fonts/font_declaration.hpp
        ek/fonts/filters_declaration.cpp
        ek/fonts/filters_declaration.hpp

        ek/xfl/flash_doc_exporter.cpp
        ek/xfl/flash_doc_exporter.hpp
        ek/xfl/export_item.cpp
        ek/xfl/export_item.hpp

        ek/xfl/animation_utility.cpp
        ek/xfl/animation_utility.hpp
        ek/flash/doc/parsing/doc_parser.cpp
        ek/flash/doc/parsing/doc_parser.hpp
        )

if(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wnull-dereference -Wpedantic -Wreturn-type -Woverloaded-virtual -Wcast-align
        -Wno-deprecated-declarations
        -Wno-gnu-anonymous-struct
        -Wno-nested-anon-types
        -fstrict-aliasing
        -Wstrict-aliasing
        )
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE
            -g -fno-exceptions)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Os
            -fno-exceptions -fno-rtti)
endif ()

target_compile_definitions(${PROJECT_NAME} PUBLIC
        -DPUGIXML_NO_XPATH)

target_link_libraries(${PROJECT_NAME}
        pugixml
        freetype2
        cairo
        scenex
        miniz
        stb
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        #UNITY_BUILD ON
        )
