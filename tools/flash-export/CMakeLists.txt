cmake_minimum_required(VERSION 3.19)
project(flash-export)

add_executable(${PROJECT_NAME}
        src/flash_export.cpp

        src/ImageSet.cpp

        # XFL Builder
        src/sxg/SGBuilder.cpp
        src/sxg/SGBuilder.hpp
        src/sxg/ExportItem.cpp
        src/sxg/ExportItem.hpp
        src/sxg/AnimationHelpers.cpp
        src/sxg/AnimationHelpers.hpp
        src/sxg/RenderElement.cpp
        src/sxg/RenderElement.hpp

        # Flash Document
        src/xfl/DocFile.cpp
        src/xfl/Doc.cpp
        src/xfl/Doc.hpp
        src/xfl/types.hpp

        src/xfl/parsing/parsing.hpp
        src/xfl/parsing/parsing.cpp
        src/xfl/parsing/BitmapDataParse.cpp
        src/xfl/parsing/DocParser.hpp
        src/xfl/parsing/DocParser.cpp

        # Rendering
       src/xfl/renderer/CairoRenderer.cpp
       src/xfl/renderer/CairoRenderer.hpp
       src/xfl/renderer/CairoHelpers.cpp
       src/xfl/renderer/CairoHelpers.hpp
       src/xfl/renderer/RenderCommand.hpp
       src/xfl/renderer/Scanner.cpp
       src/xfl/renderer/Scanner.hpp
       src/xfl/renderer/ShapeDecoder.cpp
       src/xfl/renderer/ShapeDecoder.hpp
        )

if (NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wnull-dereference -Wpedantic -Wreturn-type -Woverloaded-virtual -Wcast-align
            -Wno-deprecated-declarations
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types
            -fno-strict-aliasing
            -Wstrict-aliasing=2
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

target_compile_definitions(${PROJECT_NAME}
        PUBLIC PUGIXML_NO_XPATH
        PUBLIC PUGIXML_NO_STL
        PUBLIC PUGIXML_NO_EXCEPTIONS
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC pugixml
        PUBLIC cairo
        PUBLIC miniz
        PUBLIC ekx::sg-file
        )

target_include_directories(${PROJECT_NAME} PUBLIC src)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        #CXX_EXTENSIONS NO

        )


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g")
    else ()
        set(EKC_COMPILE_FLAGS "-Oz -flto -DNDEBUG -fno-exceptions -fno-rtti")
        set(EKC_LINK_FLAGS "-Oz -flto -Wl,-dead_strip -Wl,-dead_strip_dylibs")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EKC_COMPILE_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EKC_COMPILE_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EKC_LINK_FLAGS}")
    endif ()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME}
            PRIVATE shlwapi
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -Oz)
    endif ()
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../../cache/bin"
        RUNTIME_OUTPUT_NAME "flash-export"
        RUNTIME_OUTPUT_NAME_DEBUG "flash-export-debug"
        )
