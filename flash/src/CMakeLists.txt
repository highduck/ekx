cmake_minimum_required(VERSION 3.12)

add_library(${PROJECT_NAME}
        # flash document
        ek/flash/doc/parsing/parsing.h
        ek/flash/doc/flash_archive.cpp
        ek/flash/doc/flash_archive.h
        ek/flash/doc/flash_file.cpp
        ek/flash/doc/flash_file.h
        ek/flash/doc/bitmap.h
        ek/flash/doc/edge.h
        ek/flash/doc/filter.h
        ek/flash/doc/graphic_types.h
        ek/flash/doc/element_types.h
        ek/flash/doc/blend_mode.h
        ek/flash/doc/motion.h
        ek/flash/doc/text.h
        ek/flash/doc/dump.cpp
        ek/flash/doc/dump.h

        ek/flash/doc/parsing/bitmap.cpp
        ek/flash/doc/parsing/bitmap.h
        ek/flash/doc/parsing/filters.h
        ek/flash/doc/parsing/filters.cpp
        ek/flash/doc/parsing/basic_types.cpp
        ek/flash/doc/parsing/basic_types.h
        ek/flash/doc/parsing/graphics_parse.cpp
        ek/flash/doc/parsing/graphics_parse.h
        ek/flash/doc/parsing/elements_parse.cpp
        ek/flash/doc/parsing/elements_parse.h
        ek/flash/doc/parsing/edges.cpp
        ek/flash/doc/parsing/edges.h
        ek/flash/doc/parsing/parse_blend_mode.cpp
        ek/flash/doc/parsing/parse_blend_mode.h

        # rasterizer
        ek/flash/rasterizer/cairo_renderer.cpp
        ek/flash/rasterizer/cairo_renderer.h
        ek/flash/rasterizer/cairo_utility.cpp
        ek/flash/rasterizer/cairo_utility.h

        ek/flash/rasterizer/render_to_sprite.cpp
        ek/flash/rasterizer/render_to_sprite.h
        ek/flash/rasterizer/render_command.h
        ek/flash/rasterizer/dom_scanner.cpp
        ek/flash/rasterizer/dom_scanner.h
        ek/flash/rasterizer/shape_edge_decoder.cpp
        ek/flash/rasterizer/shape_edge_decoder.h
        ek/flash/rasterizer/shape_edge.cpp
        ek/flash/rasterizer/shape_edge.h
        ek/flash/rasterizer/shape_renderer.cpp
        ek/flash/rasterizer/shape_renderer.h
        ek/flash/rasterizer/transform_model.h
        ek/flash/rasterizer/software_filters.cpp
        ek/flash/rasterizer/software_filters.h

        ek/spritepack/sprite_data.hpp
        ek/spritepack/export_atlas.cpp
        ek/spritepack/export_atlas.hpp
        ek/spritepack/sprite_packing.cpp
        ek/spritepack/sprite_packing.hpp
        ek/spritepack/save_image_png.cpp
        ek/spritepack/save_image_png.hpp
        ek/spritepack/atlas_declaration.cpp
        ek/spritepack/atlas_declaration.hpp

        fonts/export_font.cpp
        fonts/export_font.h
        fonts/ft_library.cpp
        fonts/ft_library.h
        fonts/font_declaration.cpp
        fonts/font_declaration.hpp
        fonts/filters_declaration.cpp
        fonts/filters_declaration.hpp

        xfl/flash_doc_exporter.cpp
        xfl/flash_doc_exporter.h
        xfl/export_item.cpp
        xfl/export_item.h

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
        stb-image-write
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
