cmake_minimum_required(VERSION 3.19)
project(plugin-game-services)
add_library(${PROJECT_NAME} STATIC
        src/ek_game_services.c
        )
add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::std
        PUBLIC ekx::app
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin|iOS")

    set_source_files_properties("src/ek_game_services.c" PROPERTIES
            COMPILE_FLAGS "-x objective-c")

endif()