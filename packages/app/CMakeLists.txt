cmake_minimum_required(VERSION 3.19)
include(npm.cmake)

project(app C)

add_library(${PROJECT_NAME} STATIC
        src/app.c
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        )

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_compile_options(${PROJECT_NAME}
        PRIVATE -g
#        PRIVATE -flto
        PRIVATE -Os
        PRIVATE -Wall
        PRIVATE -Wextra
        #PRIVATE -Wno-c99-compat
        PRIVATE -std=c18
        PRIVATE -fno-strict-aliasing
        PRIVATE -fno-exceptions
        PRIVATE -fno-rtti
        PRIVATE -ffast-math
#        PRIVATE -mstackrealign
#        PRIVATE -fshort-enums
        )

target_link_options(${PROJECT_NAME}
        PRIVATE -g
#        PRIVATE -flto
        PRIVATE -Os
        PRIVATE -fno-strict-aliasing
        PRIVATE -fno-exceptions
        PRIVATE -fno-rtti
        PRIVATE -ffast-math
        )

target_link_libraries(${PROJECT_NAME}
        PRIVATE ekx::std
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    target_link_options(${PROJECT_NAME}
            PUBLIC SHELL:--js-library ${CMAKE_CURRENT_SOURCE_DIR}/src/web/app.js
            )

    target_link_libraries(${PROJECT_NAME}
            PUBLIC GLESv2
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    target_link_libraries(${PROJECT_NAME}
            PRIVATE "-framework CoreText"
            PRIVATE "-framework Foundation"
            PRIVATE "-framework UIKit"
            PRIVATE "-framework Metal"
            PRIVATE "-framework MetalKit"
            PRIVATE "-framework QuartzCore"
            )
#    target_compile_options(${PROJECT_NAME}
#            PRIVATE -fobjc-arc
#            PRIVATE -x objective-c++
#            )
    target_sources(${PROJECT_NAME} PRIVATE
            src/apple/apple.m
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${PROJECT_NAME}
            PRIVATE "-framework CoreText"
            PRIVATE "-framework Cocoa"
            PRIVATE "-framework Metal"
            PRIVATE "-framework MetalKit"
            PRIVATE "-framework QuartzCore"
            )
#    target_compile_options(${PROJECT_NAME}
#            PRIVATE -fobjc-arc
#            PRIVATE -x objective-c++
#            )
    target_sources(${PROJECT_NAME} PRIVATE
            src/apple/apple.m
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

else ()
    error("Platform: UNKNOWN")
endif ()
