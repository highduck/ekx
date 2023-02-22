cmake_minimum_required(VERSION 3.19)
project(texture-loader)

add_library(${PROJECT_NAME} STATIC
        src/texture_loader.c
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::app
        PUBLIC ekx::core
        PUBLIC ekx::sokol
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        )

target_compile_options(${PROJECT_NAME} PRIVATE
        -g
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")

    target_link_options(${PROJECT_NAME}
            PUBLIC "SHELL:--js-library ${CMAKE_CURRENT_SOURCE_DIR}/js/lib/lib-texture-loader.js"
            PUBLIC "SHELL:--pre-js ${CMAKE_CURRENT_SOURCE_DIR}/js/pre/texture-loader.js"
            )

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin|iOS")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

else ()

endif ()