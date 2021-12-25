cmake_minimum_required(VERSION 3.19)
include(npm.cmake)

project(graphics)

add_library(${PROJECT_NAME} STATIC
        src/ek/ek_gfx.c
        src/ek/draw2d/drawer.cpp
        src/ek/graphics/graphics.cpp
        src/ek/temp_res_man.c
        )

target_include_directories(${PROJECT_NAME} PUBLIC src)

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::core
        PUBLIC ekx::app
        PUBLIC sokol
        )

if (EKX_BUILD_DEV_TOOLS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SOKOL_TRACE_HOOKS)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

target_compile_options(${PROJECT_NAME} PRIVATE
        -g
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

    set_source_files_properties("src/ek/ek_gfx.c" PROPERTIES
            COMPILE_FLAGS "-x objective-c")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    set_source_files_properties("src/ek/ek_gfx.c" PROPERTIES
            COMPILE_FLAGS "-x objective-c")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

else ()

endif ()
