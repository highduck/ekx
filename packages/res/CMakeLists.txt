cmake_minimum_required(VERSION 3.19)
include(npm.cmake)
project(res)

add_library(${PROJECT_NAME} STATIC "")
add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
        src/ek/LocalResource.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC src)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::app
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

target_compile_options(${PROJECT_NAME} PRIVATE
        -g
        -fno-exceptions
        -fno-rtti
        -fstrict-aliasing
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    target_link_options(${PROJECT_NAME}
            PUBLIC SHELL:--js-library ${CMAKE_CURRENT_SOURCE_DIR}/platform/web/LocalResource.js
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    target_compile_options(${PROJECT_NAME}
            PUBLIC -x objective-c++
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_options(${PROJECT_NAME}
            PUBLIC -x objective-c++
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

else ()

endif ()