cmake_minimum_required(VERSION 3.13)
project(ek-flash)

add_library(${PROJECT_NAME}
        # Common general builders:
        ek/builders/MultiResAtlas.hpp
        ek/builders/MultiResAtlas.cpp
        ek/builders/BitmapFontBuilder.cpp
        ek/builders/BitmapFontBuilder.hpp
        ek/builders/SpriteFilters.cpp
        ek/builders/SpriteFilters.hpp

        # XFL Builder
        ek/builders/xfl/SGBuilder.cpp
        ek/builders/xfl/SGBuilder.hpp
        ek/builders/xfl/ExportItem.cpp
        ek/builders/xfl/ExportItem.hpp
        ek/builders/xfl/AnimationHelpers.cpp
        ek/builders/xfl/AnimationHelpers.hpp
        ek/builders/xfl/RenderElement.cpp
        ek/builders/xfl/RenderElement.hpp

        # Flash Document
        ek/xfl/DocFile.cpp
        ek/xfl/Doc.cpp
        ek/xfl/Doc.hpp
        ek/xfl/types.hpp

        ek/xfl/parsing/parsing.hpp
        ek/xfl/parsing/parsing.cpp
        ek/xfl/parsing/BitmapDataParse.cpp
        ek/xfl/parsing/DocParser.hpp
        ek/xfl/parsing/DocParser.cpp

        # Rendering
        ek/xfl/renderer/CairoRenderer.cpp
        ek/xfl/renderer/CairoRenderer.hpp
        ek/xfl/renderer/CairoHelpers.cpp
        ek/xfl/renderer/CairoHelpers.hpp
        ek/xfl/renderer/RenderCommand.hpp
        ek/xfl/renderer/Scanner.cpp
        ek/xfl/renderer/Scanner.hpp
        ek/xfl/renderer/ShapeDecoder.cpp
        ek/xfl/renderer/ShapeDecoder.hpp
        )

if (NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wnull-dereference -Wpedantic -Wreturn-type -Woverloaded-virtual -Wcast-align
            -Wno-deprecated-declarations
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types
            -fstrict-aliasing
            -Wstrict-aliasing
            )
endif ()
#
#if (CMAKE_BUILD_TYPE MATCHES Debug)
#    target_compile_options(${PROJECT_NAME} PRIVATE
#            -g -fno-exceptions)
#else ()
#    target_compile_options(${PROJECT_NAME} PRIVATE
#            -Os
#            -fno-exceptions -fno-rtti)
#endif ()

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
