cmake_minimum_required(VERSION 3.19)
include(npm.cmake)

project(nep)

add_library(${PROJECT_NAME} STATIC
        src/ek_nep.c
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
            PUBLIC "SHELL:--js-library ${CMAKE_CURRENT_SOURCE_DIR}/src/nep.js"
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

endif()
