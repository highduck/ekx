cmake_minimum_required(VERSION 3.19)
include(npm.cmake)

project(local-storage)

add_library(${PROJECT_NAME} STATIC
        src/local_storage.c
        src/local_storage.m
        src/nep.c
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
        PUBLIC include)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::app
        PUBLIC ekx::std
        PUBLIC ekx::core
        PUBLIC ekx::msgpack
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    target_link_options(${PROJECT_NAME}
            PUBLIC "SHELL:--js-library ${CMAKE_CURRENT_SOURCE_DIR}/src/local_storage.js"
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

endif()
